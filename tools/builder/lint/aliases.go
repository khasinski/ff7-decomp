package lint

import (
	"fmt"
	"regexp"
	"strconv"
)

// Alias is a storage alias emitted by the compiler as .set NAME, OBJECT+OFFSET.
// Read the assembly, rather than source comments, so only actual aliases qualify.
type Alias struct {
	Object string
	Offset uint32
}

var aliasRe = regexp.MustCompile(`(?m)^\s*\.set\s+(\w+)\s*,\s*(\w+)\s*(?:\+\s*(0x[0-9a-fA-F]+|[0-9]+))?\s*$`)

func scanAliases(assembly []byte) (map[string]Alias, error) {
	aliases := map[string]Alias{}
	for _, m := range aliasRe.FindAllSubmatch(assembly, -1) {
		var offset uint64
		if len(m[3]) > 0 {
			var err error
			base := 10
			value := string(m[3])
			if len(value) > 2 && value[:2] == "0x" {
				base, value = 16, value[2:]
			}
			offset, err = strconv.ParseUint(value, base, 32)
			if err != nil {
				return nil, fmt.Errorf("alias %s: invalid offset: %w", m[1], err)
			}
		}
		name := string(m[1])
		alias := Alias{Object: string(m[2]), Offset: uint32(offset)}
		if previous, ok := aliases[name]; ok && previous != alias {
			return nil, fmt.Errorf("alias %s has conflicting .set directives", name)
		}
		aliases[name] = alias
	}
	return aliases, nil
}

// applyAliases verifies addresses and complete object bounds before identifying
// shared storage. Unrelated objects at the same address remain overlap findings.
func applyAliases(symbols []Symbol, aliases map[string]Alias) error {
	byName := map[string]Symbol{}
	for _, s := range symbols {
		byName[s.Name] = s
	}
	for i := range symbols {
		s := &symbols[i]
		alias, ok := aliases[s.Name]
		if !ok {
			continue
		}
		object, ok := byName[alias.Object]
		if !ok || object.LowerBound || object.Size == 0 {
			return fmt.Errorf("alias %s needs a complete declaration of %s", s.Name, alias.Object)
		}
		if _, chained := aliases[object.Name]; chained || object.Name == s.Name {
			return fmt.Errorf("alias %s must name a storage object directly", s.Name)
		}
		address := uint64(object.Addr) + uint64(alias.Offset)
		if address != uint64(s.Addr) {
			return fmt.Errorf("alias %s: .set resolves to 0x%X, symbol table says 0x%X", s.Name, address, s.Addr)
		}
		if s.LowerBound || s.Size == 0 || uint64(alias.Offset)+uint64(s.Size) > uint64(object.Size) {
			return fmt.Errorf("alias %s lies outside %s or has an incomplete size", s.Name, object.Name)
		}
		s.AliasOf = object.Name
	}
	return nil
}

// A .set is local to its translation unit when the object is external. Every
// declaration of the alias must therefore agree about its underlying storage.
func validateAliasDeclarations(perTU [][]Symbol) error {
	seen := map[string]string{}
	for _, symbols := range perTU {
		for _, s := range symbols {
			if previous, ok := seen[s.Name]; ok && previous != s.AliasOf {
				return fmt.Errorf("symbol %s has inconsistent storage aliases across translation units", s.Name)
			}
			seen[s.Name] = s.AliasOf
		}
	}
	return nil
}

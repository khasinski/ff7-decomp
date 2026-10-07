package lint

import "testing"

func TestScanAliases(t *testing.T) {
	aliases, err := scanAliases([]byte(`
.set noreorder
.set noat
.set first, lanes+0x4
.set second, lanes + 100
.set whole, lanes
`))
	if err != nil {
		t.Fatal(err)
	}
	if len(aliases) != 3 || aliases["first"] != (Alias{"lanes", 4}) ||
		aliases["second"] != (Alias{"lanes", 100}) || aliases["whole"] != (Alias{"lanes", 0}) {
		t.Fatalf("unexpected aliases: %#v", aliases)
	}
	for _, assembly := range []string{
		".set first, lanes+0x100000000\n",
		".set first, lanes+4\n.set first, lanes+8\n",
	} {
		if _, err := scanAliases([]byte(assembly)); err == nil {
			t.Fatalf("accepted invalid alias directives: %q", assembly)
		}
	}
}

func TestStorageAliasesKeepUnrelatedOverlapFindings(t *testing.T) {
	syms := []Symbol{
		sym("lanes", 0x8009A104, 0xC0),
		sym("active", 0x8009A108, 4),
		sym("other", 0x8009A108, 4),
	}
	if err := applyAliases(syms, map[string]Alias{"active": {"lanes", 4}}); err != nil {
		t.Fatal(err)
	}
	got := findOverlaps(syms)
	if len(got) != 2 {
		t.Fatalf("expected both unrelated overlaps, got %v", got)
	}
	for _, f := range got {
		if f.A.Name != "other" && f.B.Name != "other" {
			t.Fatalf("reported shared storage as an overlap: %v", f)
		}
	}
}

func TestApplyAliasesRejectsInvalidStorage(t *testing.T) {
	cases := []struct {
		name    string
		object  Symbol
		alias   Symbol
		aliases map[string]Alias
	}{
		{"wrong address", sym("lanes", 0x8009A104, 0xC0), sym("active", 0x8009A10C, 4), map[string]Alias{"active": {"lanes", 4}}},
		{"missing object", sym("lanes", 0x8009A104, 0xC0), sym("active", 0x8009A108, 4), map[string]Alias{"active": {"missing", 4}}},
		{"out of bounds", sym("lanes", 0x8009A104, 4), sym("active", 0x8009A108, 4), map[string]Alias{"active": {"lanes", 4}}},
		{"chained alias", sym("lanes", 0x8009A104, 0xC0), sym("active", 0x8009A108, 4), map[string]Alias{"active": {"lanes", 4}, "lanes": {"root", 0}}},
		{"address overflow", sym("lanes", 0xFFFFFFFC, 8), sym("active", 0, 4), map[string]Alias{"active": {"lanes", 4}}},
	}
	for _, c := range cases {
		t.Run(c.name, func(t *testing.T) {
			if err := applyAliases([]Symbol{c.object, c.alias}, c.aliases); err == nil {
				t.Fatal("accepted invalid storage alias")
			}
		})
	}
	for _, objectIncomplete := range []bool{false, true} {
		object, alias := sym("lanes", 0x8009A104, 0xC0), sym("active", 0x8009A108, 4)
		object.LowerBound, alias.LowerBound = objectIncomplete, !objectIncomplete
		if err := applyAliases([]Symbol{object, alias}, map[string]Alias{"active": {"lanes", 4}}); err == nil {
			t.Fatal("accepted incomplete object or alias size")
		}
	}
}

func TestAliasDeclarationsMustAgreeAcrossTranslationUnits(t *testing.T) {
	alias := sym("active", 0x8009A108, 4)
	alias.AliasOf = "lanes"
	if err := validateAliasDeclarations([][]Symbol{{alias}, {alias}}); err != nil {
		t.Fatal(err)
	}
	for _, object := range []string{"", "other"} {
		other := alias
		other.AliasOf = object
		if err := validateAliasDeclarations([][]Symbol{{alias}, {other}}); err == nil {
			t.Fatal("accepted inconsistent storage declarations")
		}
	}
}

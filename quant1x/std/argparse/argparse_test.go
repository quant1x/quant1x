package argparse

import (
	"math"
	"os"
	"strings"
	"testing"
)

func TestArgumentParser_Basic(t *testing.T) {
	program := NewArgumentParser("test_prog", "1.0")
	var verbose bool
	program.AddArgument("--verbose").
		Help("enable verbose mode").
		DefaultValue(false).
		StoreInto(&verbose).
		Nargs(0)

	var name string
	program.AddArgument("--name").
		Help("name argument").
		DefaultValue("guest").
		StoreInto(&name)

	// Test default values
	if verbose != false {
		t.Errorf("expected verbose=false, got %v", verbose)
	}
	if name != "guest" {
		t.Errorf("expected name=guest, got %v", name)
	}

	// Test parsing (argparse.hpp semantics: args[0] is program name)
	args := []string{"test_prog", "--verbose", "--name", "admin"}
	err := program.ParseArgs(args)
	if err != nil {
		t.Fatalf("ParseArgs failed: %v", err)
	}

	if verbose != true {
		t.Errorf("expected verbose=true, got %v", verbose)
	}
	if name != "admin" {
		t.Errorf("expected name=admin, got %v", name)
	}
}

func TestArgumentParser_ParseArgs_ZeroPositional_SubparserDidYouMean(t *testing.T) {
	p := NewArgumentParser("git")
	notes := NewArgumentParser("notes")
	commit := NewArgumentParser("commit")
	p.AddSubparser(notes)
	p.AddSubparser(commit)

	err := p.ParseArgs([]string{"git", "totes"})
	if err == nil {
		t.Fatalf("expected error")
	}
	if err.Error() != "Failed to parse 'totes', did you mean 'notes'" {
		t.Fatalf("unexpected error: %q", err.Error())
	}
}

func TestArgumentParser_ParseArgs_ZeroPositional_OptionalDidYouMean(t *testing.T) {
	p := NewArgumentParser("prog")
	p.AddArgument("-n", "--name")

	err := p.ParseArgs([]string{"prog", "foo"})
	if err == nil {
		t.Fatalf("expected error")
	}
	if err.Error() != "Zero positional arguments expected, did you mean -n/--name VAR" {
		t.Fatalf("unexpected error: %q", err.Error())
	}
}

func TestArgumentParser_ParseArgs_ZeroPositional_FallbackNoSuggestion(t *testing.T) {
	p := NewArgumentParser("prog")
	p.AddArgument("--flag").ImplicitValue(true).Nargs(0)

	err := p.ParseArgs([]string{"prog", "foo"})
	if err == nil {
		t.Fatalf("expected error")
	}
	if err.Error() != "Zero positional arguments expected" {
		t.Fatalf("unexpected error: %q", err.Error())
	}
}

func TestArgumentParser_FormatHelp_ShowsOptionalHeaderEvenIfAllHidden(t *testing.T) {
	p := NewArgumentParser("prog")
	p.AddArgument("--hidden").Hidden()

	h := p.FormatHelp()
	if !strings.Contains(h, "Optional arguments:\n") {
		t.Fatalf("expected Optional arguments header, got: %q", h)
	}
}

func TestArgumentParser_FormatHelp_ShowsGroupHeaderEvenIfNoVisibleArgs(t *testing.T) {
	p := NewArgumentParser("prog")
	p.AddGroup("Group1")
	p.AddArgument("--g").Hidden()

	h := p.FormatHelp()
	if !strings.Contains(h, "\nGroup1 (detailed usage):\n") {
		t.Fatalf("expected group header, got: %q", h)
	}
}

func TestReprAny_FloatDefaultPrecision(t *testing.T) {
	p := NewArgumentParser("prog")
	a := p.AddArgument("--f").DefaultValue(1.23456789)
	h := p.FormatHelp()
	if !strings.Contains(h, "[default: 1.23457]") {
		t.Fatalf("expected rounded float default repr in help, got: %q", h)
	}
	_ = a
}

func TestReprAny_FloatNaNInf(t *testing.T) {
	p := NewArgumentParser("prog")
	p.AddArgument("--nan").DefaultValue(math.NaN())
	p.AddArgument("--inf").DefaultValue(math.Inf(1))
	p.AddArgument("--ninf").DefaultValue(math.Inf(-1))
	h := p.FormatHelp()
	if !strings.Contains(h, "[default: nan]") {
		t.Fatalf("expected nan, got: %q", h)
	}
	if !strings.Contains(h, "[default: inf]") {
		t.Fatalf("expected inf, got: %q", h)
	}
	if !strings.Contains(h, "[default: -inf]") {
		t.Fatalf("expected -inf, got: %q", h)
	}
}

func TestReprAny_Uint8Int8AsChar(t *testing.T) {
	p := NewArgumentParser("prog")
	p.AddArgument("--b").DefaultValue(uint8(97))
	p.AddArgument("--c").DefaultValue(int8(98))
	h := p.FormatHelp()
	if !strings.Contains(h, "[default: a]") {
		t.Fatalf("expected uint8 as char a, got: %q", h)
	}
	if !strings.Contains(h, "[default: b]") {
		t.Fatalf("expected int8 as char b, got: %q", h)
	}
}

func TestReprAny_NotRepresentable(t *testing.T) {
	p := NewArgumentParser("prog")
	p.AddArgument("--x").DefaultValue(struct{}{})
	h := p.FormatHelp()
	if !strings.Contains(h, "[default: <not representable>]") {
		t.Fatalf("expected <not representable>, got: %q", h)
	}
}

func TestReprAny_ContainerTruncation(t *testing.T) {
	p := NewArgumentParser("prog")
	p.AddArgument("--xs").DefaultValue([]int{1, 2, 3, 4, 5, 6})
	h := p.FormatHelp()
	if !strings.Contains(h, "[default: {1 2 3 4...6}]") {
		t.Fatalf("expected truncated container repr, got: %q", h)
	}
}

func TestArgumentParser_FormatHelp_WidthIncludesHiddenArgsAndSuppressedSubcommands(t *testing.T) {
	p := NewArgumentParser("prog")
	p.AddArgument("-a").Help("A").Nargs(0)
	p.AddArgument("-Z", "--this-is-a-very-long-hidden-option-name").Help("hidden").Nargs(0).Hidden()

	run := NewArgumentParser("run")
	run.AddDescription("run it")
	p.AddSubparser(run)

	suppressedName := "this-is-a-very-very-very-long-suppressed-subcommand-name"
	sup := NewArgumentParser(suppressedName)
	sup.AddDescription("should not show")
	sup.suppress = true
	p.AddSubparser(sup)

	h := p.FormatHelp()
	if strings.Contains(h, suppressedName) {
		t.Fatalf("did not expect suppressed subcommand name in help, got: %q", h)
	}
	if !strings.Contains(h, "Subcommands:\n") {
		t.Fatalf("expected Subcommands section, got: %q", h)
	}

	hiddenNamesSize := len("-Z") + len("--this-is-a-very-long-hidden-option-name")
	hiddenArgLen := hiddenNamesSize + 2*(2-1) + 2
	width := hiddenArgLen
	if len(suppressedName) > width {
		width = len(suppressedName)
	}
	colw := width - 2
	if colw < 0 {
		colw = 0
	}
	expectedRunLine := "  " + padRight("run", colw) + " " + "run it" + "\n"
	if !strings.Contains(h, expectedRunLine) {
		t.Fatalf("expected padded subcommand line %q in help, got: %q", expectedRunLine, h)
	}
}

func TestArgumentParser_FormatHelp_Golden(t *testing.T) {
	p := NewArgumentParser("prog")
	p.AddDescription("desc")
	p.AddEpilog("epilog")

	p.AddArgument("input").Help("input file")
	p.AddArgument("output").Help("output file\nsecond line")

	p.AddArgument("-v", "--verbose").Help("verbose").Nargs(0)

	p.AddGroup("Group1")
	p.AddArgument("--mode").Metavar("MODE").Help("mode select")

	p.AddArgument("--this-is-a-very-long-hidden-option-name").Help("hidden").Nargs(0).Hidden()

	run := NewArgumentParser("run")
	run.AddDescription("run it")
	p.AddSubparser(run)

	suppressedName := "this-is-a-very-very-very-long-suppressed-subcommand-name"
	sup := NewArgumentParser(suppressedName)
	sup.AddDescription("should not show")
	sup.suppress = true
	p.AddSubparser(sup)

	h := p.FormatHelp()

	width := len(suppressedName)
	pad := func(s string) string {
		if len(s) >= width {
			return s
		}
		return s + strings.Repeat(" ", width-len(s))
	}

	var sb strings.Builder
	sb.WriteString("Usage: prog [--help] [--verbose] [--mode MODE] input output {run}\n\n")
	sb.WriteString("desc\n\n")

	sb.WriteString("Positional arguments:\n")
	sb.WriteString(pad("  input") + "  " + "input file" + " " + "\n")
	sb.WriteString(pad("  output") + "  " + "output file\n")
	sb.WriteString(pad(strings.Repeat(" ", len("  output"))) + "  " + "second line" + " " + "\n")

	sb.WriteString("\n")
	sb.WriteString("Optional arguments:\n")
	sb.WriteString(pad("  -h, --help") + "  " + "shows help message and exits" + " " + "\n")
	sb.WriteString(pad("  -v, --verbose") + "  " + "verbose" + " " + "\n")

	sb.WriteString("\n")
	sb.WriteString("Group1 (detailed usage):\n")
	sb.WriteString(pad("  --mode MODE") + "  " + "mode select" + " " + "\n")

	sb.WriteString("\n")
	sb.WriteString("Subcommands:\n")
	colw := width - 2
	if colw < 0 {
		colw = 0
	}
	padw := colw - len("run")
	if padw < 0 {
		padw = 0
	}
	sb.WriteString("  " + "run" + strings.Repeat(" ", padw) + " " + "run it" + "\n")

	sb.WriteString("\n")
	sb.WriteString("epilog\n\n")

	expected := sb.String()
	if h != expected {
		t.Fatalf("unexpected help output\n--- expected ---\n%q\n--- got ---\n%q", expected, h)
	}
}

func TestArgumentParser_FormatHelp_Golden_HiddenPositionalAffectsSubcommandsSpacing(t *testing.T) {
	p := NewArgumentParser("prog")
	longHiddenPos := "this-is-a-very-long-hidden-positional-name"
	p.AddArgument(longHiddenPos).Hidden()
	p.AddArgument("-x").Help("x").Nargs(0)

	run := NewArgumentParser("run")
	run.AddDescription("run it")
	p.AddSubparser(run)

	h := p.FormatHelp()

	width := 2 + len(longHiddenPos)
	pad := func(s string) string {
		if len(s) >= width {
			return s
		}
		return s + strings.Repeat(" ", width-len(s))
	}

	var sb strings.Builder
	sb.WriteString("Usage: prog [--help] [-x] {run}\n\n")
	sb.WriteString("Optional arguments:\n")
	sb.WriteString(pad("  -h, --help") + "  " + "shows help message and exits" + " " + "\n")
	sb.WriteString(pad("  -x") + "  " + "x" + " " + "\n")

	sb.WriteString("\n")
	sb.WriteString("Subcommands:\n")
	colw := width - 2
	if colw < 0 {
		colw = 0
	}
	padw := colw - len("run")
	if padw < 0 {
		padw = 0
	}
	sb.WriteString("  " + "run" + strings.Repeat(" ", padw) + " " + "run it" + "\n")

	expected := sb.String()
	if h != expected {
		t.Fatalf("unexpected help output\n--- expected ---\n%q\n--- got ---\n%q", expected, h)
	}
}

func TestArgumentParser_FormatHelp_Golden_NargsAndDefaultReprSpacing(t *testing.T) {
	p := NewArgumentParser("prog")
	p.AddArgument("--pair").Help("pair").Nargs(2).DefaultValue([]string{"a", "b"})

	h := p.FormatHelp()

	width := len("  -h, --help")
	if len("  --pair") > width {
		width = len("  --pair")
	}
	pad := func(s string) string {
		if len(s) >= width {
			return s
		}
		return s + strings.Repeat(" ", width-len(s))
	}

	var sb strings.Builder
	sb.WriteString("Usage: prog [--help] [--pair VAR...]\n\n")
	sb.WriteString("Optional arguments:\n")
	sb.WriteString(pad("  -h, --help") + "  " + "shows help message and exits" + " " + "\n")
	sb.WriteString(pad("  --pair") + "  " + "pair" + " " + "[nargs=0..2] " + "[default: {\"a\" \"b\"}]" + "\n")

	expected := sb.String()
	if h != expected {
		t.Fatalf("unexpected help output\n--- expected ---\n%q\n--- got ---\n%q", expected, h)
	}
}

func TestArgumentParser_FormatHelp_Golden_MetavarContainsAngleSpaceAngle(t *testing.T) {
	p := NewArgumentParser("prog")
	p.AddArgument("--pair").Metavar("<A> <B>").Help("pair").Nargs(2)

	h := p.FormatHelp()

	width := len("  -h, --help")
	// get_arguments_length() does not include this metavar for nargs!=1, but
	// the name stream does print it; width should remain the max of other args.
	pad := func(s string) string {
		if len(s) >= width {
			return s
		}
		return s + strings.Repeat(" ", width-len(s))
	}

	var sb strings.Builder
	sb.WriteString("Usage: prog [--help] [--pair <A> <B>]\n\n")
	sb.WriteString("Optional arguments:\n")
	sb.WriteString(pad("  -h, --help") + "  " + "shows help message and exits" + " " + "\n")
	// name stream includes metavar even though padding width doesn't.
	sb.WriteString("  --pair <A> <B>" + "  " + "pair" + " " + "[nargs: 2] " + "\n")

	expected := sb.String()
	if h != expected {
		t.Fatalf("unexpected help output\n--- expected ---\n%q\n--- got ---\n%q", expected, h)
	}
}

func TestArgumentParser_FormatHelp_Golden_DefaultVersionArgument(t *testing.T) {
	p := NewArgumentParser("prog", "1.2.3")
	h := p.FormatHelp()

	width := len("  -v, --version")
	if len("  -h, --help") > width {
		width = len("  -h, --help")
	}
	pad := func(s string) string {
		if len(s) >= width {
			return s
		}
		return s + strings.Repeat(" ", width-len(s))
	}

	var sb strings.Builder
	sb.WriteString("Usage: prog [--help] [--version]\n\n")
	sb.WriteString("Optional arguments:\n")
	sb.WriteString(pad("  -h, --help") + "  " + "shows help message and exits" + " " + "\n")
	sb.WriteString(pad("  -v, --version") + "  " + "prints version information and exits" + " " + "\n")

	expected := sb.String()
	if h != expected {
		t.Fatalf("unexpected help output\n--- expected ---\n%q\n--- got ---\n%q", expected, h)
	}
}

func TestArgumentParser_Usage_Golden_MutexWrapBreakOnMutex(t *testing.T) {
	p := NewArgumentParser("prog")
	p.SetUsageMaxLineWidth(30)
	p.SetUsageBreakOnMutex(true)

	p.AddArgument("-a", "--alpha").Nargs(0)

	g := p.AddMutuallyExclusiveGroup(false)
	g.AddArgument("--foo").Nargs(0)
	g.AddArgument("--bar").Nargs(0)

	// add a non-mutex option after the mutex group to force break-on-mutex flush
	p.AddArgument("--baz").Nargs(0)

	p.AddArgument("input")

	expected := "Usage: prog [--help] [--alpha]\n" +
		"            [[--foo]|[--bar]]\n" +
		"            [--baz]\n" +
		"            input"

	if got := p.Usage(); got != expected {
		t.Fatalf("unexpected usage\n--- expected ---\n%q\n--- got ---\n%q", expected, got)
	}
}

func TestArgumentParser_Usage_Golden_AddUsageNewline(t *testing.T) {
	p := NewArgumentParser("prog")
	p.SetUsageMaxLineWidth(40)

	p.AddArgument("--a").Nargs(0)
	p.AddArgument("--b").Nargs(0)
	p.AddUsageNewline()
	p.AddArgument("--c").Nargs(0)
	p.AddArgument("--d").Nargs(0)

	expected := "Usage: prog [--help] [--a] [--b]\n" +
		"            [--c] [--d]"

	if got := p.Usage(); got != expected {
		t.Fatalf("unexpected usage\n--- expected ---\n%q\n--- got ---\n%q", expected, got)
	}
}

func TestArgumentParser_Usage_Golden_GroupsWithWrappingAndUsageNewline(t *testing.T) {
	p := NewArgumentParser("prog")
	p.SetUsageMaxLineWidth(25)

	// Group 0 option (plus default --help)
	p.AddArgument("--a").Nargs(0)

	p.AddGroup("Group1")
	p.AddArgument("--g1").Nargs(0)
	p.AddUsageNewline()
	p.AddArgument("--g2").Nargs(0)

	p.AddGroup("Group2")
	p.AddArgument("--h1").Nargs(0)

	p.AddArgument("input")

	indent := strings.Repeat(" ", len("Usage: prog"))
	indent1 := indent + " "

	expected := "Usage: prog [--help]\n" +
		indent1 + "[--a]\n" +
		indent1 + "input\n\n" +
		"Group1:\n" +
		indent1 + "[--g1]\n" +
		indent1 + "[--g2]\n\n" +
		"Group2:\n" +
		indent1 + "[--h1]"

	if got := p.Usage(); got != expected {
		t.Fatalf("unexpected usage\n--- expected ---\n%q\n--- got ---\n%q", expected, got)
	}
}

func TestArgumentParser_Usage_Golden_AllSubcommandsSuppressedShowsEmptyBraces(t *testing.T) {
	p := NewArgumentParser("prog")

	s1 := NewArgumentParser("one")
	s1.AddDescription("one")
	s1.suppress = true
	p.AddSubparser(s1)

	s2 := NewArgumentParser("two")
	s2.AddDescription("two")
	s2.suppress = true
	p.AddSubparser(s2)

	expected := "Usage: prog [--help] {}"
	if got := p.Usage(); got != expected {
		t.Fatalf("unexpected usage\n--- expected ---\n%q\n--- got ---\n%q", expected, got)
	}
}

func TestArgumentParser_FormatHelp_Golden_AllSubcommandsSuppressedNoSection(t *testing.T) {
	p := NewArgumentParser("prog")

	s1 := NewArgumentParser("one")
	s1.AddDescription("one")
	s1.suppress = true
	p.AddSubparser(s1)

	h := p.FormatHelp()

	// Usage must still include empty braces, but help must not include a
	// Subcommands section since none are visible.
	if !strings.Contains(h, "Usage: prog [--help] {}\n\n") {
		t.Fatalf("expected usage to include empty braces, got: %q", h)
	}
	if strings.Contains(h, "Subcommands:\n") {
		t.Fatalf("did not expect Subcommands section, got: %q", h)
	}
}

func TestArgumentParser_Subcommand(t *testing.T) {
	program := NewArgumentParser("git")

	commitCmd := NewArgumentParser("commit")
	var message string
	commitCmd.AddArgument("-m").StoreInto(&message)
	program.AddSubparser(commitCmd)

	addCmd := NewArgumentParser("add")
	var all bool
	addCmd.AddArgument("-A").StoreInto(&all).Nargs(0)
	program.AddSubparser(addCmd)

	// Test commit subcommand
	args := []string{"git", "commit", "-m", "initial commit"}
	err := program.ParseArgs(args)
	if err != nil {
		t.Fatalf("ParseArgs failed: %v", err)
	}

	if !program.IsSubcommandUsed("commit") {
		t.Error("expected commit subcommand to be used")
	}
	if message != "initial commit" {
		t.Errorf("expected message='initial commit', got '%v'", message)
	}

	// Test add subcommand
	args2 := []string{"git", "add", "-A"}
	err = program.ParseArgs(args2)
	if err != nil {
		t.Fatalf("ParseArgs failed: %v", err)
	}

	if !program.IsSubcommandUsed("add") {
		t.Error("expected add subcommand to be used")
	}
	if !all {
		t.Error("expected all=true")
	}
}

func TestArgumentParser_Positional(t *testing.T) {
	program := NewArgumentParser("cp")
	var src string
	var dest string

	program.AddArgument("src").StoreInto(&src)
	program.AddArgument("dest").StoreInto(&dest)

	args := []string{"cp", "file1.txt", "file2.txt"}
	err := program.ParseArgs(args)
	if err != nil {
		t.Fatalf("ParseArgs failed: %v", err)
	}

	if src != "file1.txt" {
		t.Errorf("expected src='file1.txt', got '%v'", src)
	}
	if dest != "file2.txt" {
		t.Errorf("expected dest='file2.txt', got '%v'", dest)
	}
}

func TestArgumentParser_Choices(t *testing.T) {
	program := NewArgumentParser("test")
	var mode string
	program.AddArgument("--mode").
		Choices("fast", "slow").
		StoreInto(&mode)

	// Valid choice
	err := program.ParseArgs([]string{"test", "--mode", "fast"})
	if err != nil {
		t.Errorf("unexpected error: %v", err)
	}
	if mode != "fast" {
		t.Errorf("expected mode='fast', got '%v'", mode)
	}

	// Invalid choice
	err = program.ParseArgs([]string{"test", "--mode", "medium"})
	if err == nil {
		t.Error("expected error for invalid choice")
	}
}

func TestArgumentParser_ParseArgsOS(t *testing.T) {
	old := os.Args
	defer func() { os.Args = old }()

	program := NewArgumentParser("p")
	var name string
	program.AddArgument("--name").StoreInto(&name)

	os.Args = []string{"p", "--name", "x"}
	if err := program.ParseArgsOS(); err != nil {
		t.Fatalf("ParseArgsOS failed: %v", err)
	}
	if name != "x" {
		t.Fatalf("expected name=x, got %q", name)
	}
}

func TestArgumentParser_FormatHelp_Sections(t *testing.T) {
	p := NewArgumentParser("prog")
	p.AddDescription("desc")
	p.AddArgument("input").Help("input file")
	p.AddArgument("-n", "--name").Help("user name").Metavar("NAME")

	h := p.FormatHelp()
	if !strings.Contains(h, "Usage: prog") {
		t.Fatalf("expected usage header, got: %q", h)
	}
	if !strings.Contains(h, "Positional arguments:\n") {
		t.Fatalf("expected positional section, got: %q", h)
	}
	if !strings.Contains(h, "Optional arguments:\n") {
		t.Fatalf("expected optional section, got: %q", h)
	}
}

func TestArgumentParser_Usage_WrapsWhenMaxLineWidthSet(t *testing.T) {
	p := NewArgumentParser("prog")
	p.SetUsageMaxLineWidth(20)
	p.AddArgument("-a", "--alpha").Help("alpha")
	p.AddArgument("-b", "--beta").Help("beta")
	p.AddArgument("-c", "--charlie").Help("charlie")

	u := p.Usage()
	if !strings.Contains(u, "\n") {
		t.Fatalf("expected wrapped usage to contain newline, got: %q", u)
	}
}

func TestArgumentParser_Usage_MutexUsesPipeNoSpaces(t *testing.T) {
	p := NewArgumentParser("prog")
	// Enable multiline path (usage_newline_counter + mutex logic), but keep width large.
	p.SetUsageMaxLineWidth(200)
	g := p.AddMutuallyExclusiveGroup(false)
	g.AddArgument("-a").Nargs(0)
	g.AddArgument("-b").Nargs(0)

	u := p.Usage()
	// argparse.hpp outputs nested brackets when args are not required:
	// Usage: prog [[-a]|[-b]]
	if !strings.Contains(u, "[[-a]|[-b]]") {
		t.Fatalf("expected mutex usage with '|' and no spaces, got: %q", u)
	}
}

func TestArgumentParser_Usage_NewlineCounterBreaksLine(t *testing.T) {
	p := NewArgumentParser("prog")
	// Enable multiline path without forcing wrap.
	p.SetUsageMaxLineWidth(200)
	p.AddArgument("--first").Nargs(0)
	p.AddUsageNewline()
	p.AddArgument("--second").Nargs(0)

	u := p.Usage()
	if !strings.Contains(u, "[--first]\n") {
		t.Fatalf("expected newline after first option, got: %q", u)
	}
	if !strings.Contains(u, "\n") {
		t.Fatalf("expected multiline usage, got: %q", u)
	}
}

func TestArgument_FormatHelp_MetavarShownOnlyForCertainNargs(t *testing.T) {
	{
		p := NewArgumentParser("prog")
		p.AddArgument("-n", "--name").Metavar("NAME").Help("user").Nargs(1)
		h := p.FormatHelp()
		if !strings.Contains(h, "-n, --name NAME") {
			t.Fatalf("expected metavar to be shown for nargs==1, got: %q", h)
		}
	}
	{
		p := NewArgumentParser("prog")
		p.AddArgument("-n", "--name").Metavar("NAME").Help("user").NargsPattern(NArgsOptional)
		h := p.FormatHelp()
		if strings.Contains(h, "-n, --name NAME") {
			t.Fatalf("expected metavar not to be shown for nargs!=1 unless special, got: %q", h)
		}
		if !strings.Contains(h, "-n, --name") {
			t.Fatalf("expected names to be present, got: %q", h)
		}
	}
}

func TestArgument_FormatHelp_DefaultUsesRepr(t *testing.T) {
	p := NewArgumentParser("prog")
	p.AddArgument("--mode").Help("mode").DefaultValue("fast")
	h := p.FormatHelp()
	if !strings.Contains(h, "[default: \"fast\"]") {
		t.Fatalf("expected quoted string default repr, got: %q", h)
	}
}

func TestArgument_FormatHelp_DefaultBoolUsesTrueFalse(t *testing.T) {
	p := NewArgumentParser("prog")
	p.AddArgument("--flag").Help("flag").DefaultValue(true).Nargs(1)
	h := p.FormatHelp()
	if !strings.Contains(h, "[default: true]") {
		t.Fatalf("expected bool default repr true, got: %q", h)
	}
}

func TestArgument_FormatHelp_DefaultSliceUsesContainerRepr(t *testing.T) {
	p := NewArgumentParser("prog")
	p.AddArgument("--vals").Help("vals").DefaultValue([]string{"a", "b"})
	h := p.FormatHelp()
	if !strings.Contains(h, "[default: {\"a\" \"b\"}]") {
		t.Fatalf("expected container repr for default slice, got: %q", h)
	}
}

func TestArgument_Scan_AutoIntegerBase(t *testing.T) {
	p := NewArgumentParser("prog")
	p.AddArgument("--n").Scan('i')

	if err := p.ParseArgs([]string{"prog", "--n", "0x10"}); err != nil {
		t.Fatalf("ParseArgs failed: %v", err)
	}
	var n int
	if err := p.GetInto("--n", &n); err != nil {
		t.Fatalf("GetInto failed: %v", err)
	}
	if n != 16 {
		t.Fatalf("expected n=16, got %d", n)
	}
}

func TestArgument_Scan_AutoIntegerBase_HexTrailingJunkDoesNotMatchToEnd(t *testing.T) {
	p := NewArgumentParser("prog")
	p.AddArgument("--n").Scan('i')
	err := p.ParseArgs([]string{"prog", "--n", "0x1g"})
	if err == nil {
		t.Fatalf("expected error")
	}
	// argparse.hpp wraps radix_16 errors: "Failed to parse '<s>' as hexadecimal: <inner>".
	if !strings.Contains(err.Error(), "Failed to parse '0x1g' as hexadecimal") {
		t.Fatalf("unexpected error: %v", err)
	}
	if !strings.Contains(err.Error(), "pattern '1g' does not match to the end") {
		t.Fatalf("unexpected error: %v", err)
	}
}

func TestArgument_Scan_AutoIntegerBase_OctalTrailingJunkDoesNotMatchToEnd(t *testing.T) {
	p := NewArgumentParser("prog")
	p.AddArgument("--n").Scan('i')
	err := p.ParseArgs([]string{"prog", "--n", "09"})
	if err == nil {
		t.Fatalf("expected error")
	}
	// Leading 0 selects octal in scan('i'), and errors are wrapped.
	if !strings.Contains(err.Error(), "Failed to parse '09' as octal") {
		t.Fatalf("unexpected error: %v", err)
	}
	if !strings.Contains(err.Error(), "pattern '09' does not match to the end") {
		t.Fatalf("unexpected error: %v", err)
	}
}

func TestArgument_Scan_AutoIntegerBase_DecimalTrailingJunkDoesNotMatchToEnd(t *testing.T) {
	p := NewArgumentParser("prog")
	p.AddArgument("--n").Scan('i')
	err := p.ParseArgs([]string{"prog", "--n", "12x"})
	if err == nil {
		t.Fatalf("expected error")
	}
	if !strings.Contains(err.Error(), "Failed to parse '12x' as decimal integer") {
		t.Fatalf("unexpected error: %v", err)
	}
	if !strings.Contains(err.Error(), "pattern '12x' does not match to the end") {
		t.Fatalf("unexpected error: %v", err)
	}
}

func TestArgument_Scan_BinaryUnsignedRequiresPrefix(t *testing.T) {
	p := NewArgumentParser("prog")
	p.AddArgument("--u").Scan('b')
	if err := p.ParseArgs([]string{"prog", "--u", "101"}); err == nil {
		t.Fatalf("expected error when missing 0b prefix")
	}
}

func TestArgument_Scan_BinaryUnsignedTrailingJunkDoesNotMatchToEnd(t *testing.T) {
	p := NewArgumentParser("prog")
	p.AddArgument("--u").Scan('b')
	err := p.ParseArgs([]string{"prog", "--u", "0b102"})
	if err == nil {
		t.Fatalf("expected error")
	}
	// radix_2 scan parses on rest, so error references the rest string.
	if !strings.Contains(err.Error(), "pattern '102' does not match to the end") {
		t.Fatalf("unexpected error: %v", err)
	}
}

func TestArgument_Scan_BinaryUnsignedRangeError(t *testing.T) {
	p := NewArgumentParser("prog")
	p.AddArgument("--u").Scan('b')

	// 2^64 in binary, with 0b prefix.
	rest := "1" + strings.Repeat("0", 64)
	err := p.ParseArgs([]string{"prog", "--u", "0b" + rest})
	if err == nil {
		t.Fatalf("expected error")
	}
	// radix_2 scan parses on rest, so range error references rest.
	if !strings.Contains(err.Error(), "not representable") {
		t.Fatalf("unexpected error: %v", err)
	}
}

func TestArgument_Scan_OctalUnsignedTrailingJunkDoesNotMatchToEnd(t *testing.T) {
	p := NewArgumentParser("prog")
	p.AddArgument("--u").Scan('o')
	err := p.ParseArgs([]string{"prog", "--u", "18"})
	if err == nil {
		t.Fatalf("expected error")
	}
	// radix_8 scan does not wrap; error is directly from strict parser.
	if !strings.Contains(err.Error(), "pattern '18' does not match to the end") {
		t.Fatalf("unexpected error: %v", err)
	}
}

func TestArgument_Scan_OctalUnsignedRangeError(t *testing.T) {
	p := NewArgumentParser("prog")
	p.AddArgument("--u").Scan('o')

	// 2^64 in octal (radix_8), should overflow uint64.
	err := p.ParseArgs([]string{"prog", "--u", "2000000000000000000000"})
	if err == nil {
		t.Fatalf("expected error")
	}
	if !strings.Contains(err.Error(), "not representable") {
		t.Fatalf("unexpected error: %v", err)
	}
}

func TestArgument_Scan_HexUnsignedTrailingJunkDoesNotMatchToEnd(t *testing.T) {
	p := NewArgumentParser("prog")
	p.AddArgument("--u").Scan('x')
	err := p.ParseArgs([]string{"prog", "--u", "1g"})
	if err == nil {
		t.Fatalf("expected error")
	}
	if !strings.Contains(err.Error(), "Failed to parse '1g' as hexadecimal") {
		t.Fatalf("unexpected error: %v", err)
	}
	if !strings.Contains(err.Error(), "pattern '1g' does not match to the end") {
		t.Fatalf("unexpected error: %v", err)
	}
}

func TestArgument_Scan_HexUnsignedNotFound(t *testing.T) {
	p := NewArgumentParser("prog")
	p.AddArgument("--u").Scan('x')
	err := p.ParseArgs([]string{"prog", "--u", "g1"})
	if err == nil {
		t.Fatalf("expected error")
	}
	if !strings.Contains(err.Error(), "Failed to parse 'g1' as hexadecimal") {
		t.Fatalf("unexpected error: %v", err)
	}
	if !strings.Contains(err.Error(), "pattern 'g1' not found") {
		t.Fatalf("unexpected error: %v", err)
	}
}

func TestArgument_Scan_HexUnsignedRangeError(t *testing.T) {
	p := NewArgumentParser("prog")
	p.AddArgument("--u").Scan('x')
	err := p.ParseArgs([]string{"prog", "--u", "10000000000000000"})
	if err == nil {
		t.Fatalf("expected error")
	}
	if !strings.Contains(err.Error(), "Failed to parse '10000000000000000' as hexadecimal") {
		t.Fatalf("unexpected error: %v", err)
	}
	if !strings.Contains(err.Error(), "not representable") {
		t.Fatalf("unexpected error: %v", err)
	}
}

func TestArgument_Scan_HexUnsignedPrefixTrailingJunkDoesNotMatchToEnd(t *testing.T) {
	p := NewArgumentParser("prog")
	p.AddArgument("--u").Scan('x')
	err := p.ParseArgs([]string{"prog", "--u", "0x1g"})
	if err == nil {
		t.Fatalf("expected error")
	}
	if !strings.Contains(err.Error(), "Failed to parse '0x1g' as hexadecimal") {
		t.Fatalf("unexpected error: %v", err)
	}
	// For prefixed hex, parse happens on the rest, so inner error references rest.
	if !strings.Contains(err.Error(), "pattern '1g' does not match to the end") {
		t.Fatalf("unexpected error: %v", err)
	}
}

func TestArgument_Scan_HexUnsignedPrefixRangeError(t *testing.T) {
	p := NewArgumentParser("prog")
	p.AddArgument("--u").Scan('x')
	err := p.ParseArgs([]string{"prog", "--u", "0x10000000000000000"})
	if err == nil {
		t.Fatalf("expected error")
	}
	if !strings.Contains(err.Error(), "Failed to parse '0x10000000000000000' as hexadecimal") {
		t.Fatalf("unexpected error: %v", err)
	}
	if !strings.Contains(err.Error(), "not representable") {
		t.Fatalf("unexpected error: %v", err)
	}
}

func TestArgument_Scan_ScientificRequiresExponent(t *testing.T) {
	p := NewArgumentParser("prog")
	p.AddArgument("--f").Scan('e')
	if err := p.ParseArgs([]string{"prog", "--f", "1.25"}); err == nil {
		t.Fatalf("expected error for missing exponent part")
	}
}

func TestArgument_GetInto_ScanInt8RangeError(t *testing.T) {
	{
		p := NewArgumentParser("prog")
		p.AddArgument("--n").Scan('i')
		if err := p.ParseArgs([]string{"prog", "--n", "127"}); err != nil {
			t.Fatalf("ParseArgs failed: %v", err)
		}
		var ok int8
		if err := p.GetInto("--n", &ok); err != nil {
			t.Fatalf("GetInto failed: %v", err)
		}
		if ok != 127 {
			t.Fatalf("expected 127, got %d", ok)
		}
	}

	{
		p := NewArgumentParser("prog")
		p.AddArgument("--n").Scan('i')
		if err := p.ParseArgs([]string{"prog", "--n", "128"}); err != nil {
			t.Fatalf("ParseArgs failed: %v", err)
		}
		var bad int8
		if err := p.GetInto("--n", &bad); err == nil {
			t.Fatalf("expected range error converting to int8")
		}
	}
}

func TestArgument_GetInto_ScanUint8RangeError(t *testing.T) {
	{
		p := NewArgumentParser("prog")
		p.AddArgument("--u").Scan('u')
		if err := p.ParseArgs([]string{"prog", "--u", "255"}); err != nil {
			t.Fatalf("ParseArgs failed: %v", err)
		}
		var ok uint8
		if err := p.GetInto("--u", &ok); err != nil {
			t.Fatalf("GetInto failed: %v", err)
		}
		if ok != 255 {
			t.Fatalf("expected 255, got %d", ok)
		}
	}

	{
		p := NewArgumentParser("prog")
		p.AddArgument("--u").Scan('u')
		if err := p.ParseArgs([]string{"prog", "--u", "256"}); err != nil {
			t.Fatalf("ParseArgs failed: %v", err)
		}
		var bad uint8
		if err := p.GetInto("--u", &bad); err == nil {
			t.Fatalf("expected range error converting to uint8")
		}
	}
}

func TestArgument_GetInto_ScanFloat32RangeError(t *testing.T) {
	{
		p := NewArgumentParser("prog")
		p.AddArgument("--f").Scan('g')
		if err := p.ParseArgs([]string{"prog", "--f", "3.5"}); err != nil {
			t.Fatalf("ParseArgs failed: %v", err)
		}
		var ok float32
		if err := p.GetInto("--f", &ok); err != nil {
			t.Fatalf("GetInto failed: %v", err)
		}
		if ok != float32(3.5) {
			t.Fatalf("expected 3.5, got %v", ok)
		}
	}

	{
		p := NewArgumentParser("prog")
		p.AddArgument("--f").Scan('g')
		if err := p.ParseArgs([]string{"prog", "--f", "1e50"}); err != nil {
			t.Fatalf("ParseArgs failed: %v", err)
		}
		var bad float32
		if err := p.GetInto("--f", &bad); err == nil {
			t.Fatalf("expected range error converting to float32")
		}
	}
}

func TestArgument_StoreInto_FloatGeneralRejectsHexFloat(t *testing.T) {
	p := NewArgumentParser("prog")
	var f float64
	p.AddArgument("--f").StoreInto(&f)
	if err := p.ParseArgs([]string{"prog", "--f", "0x1p0"}); err == nil {
		t.Fatalf("expected error for hexfloat under general float parsing")
	}
}

func TestArgument_StoreInto_IntRejectsLeadingPlus(t *testing.T) {
	p := NewArgumentParser("prog")
	var n int
	p.AddArgument("--n").StoreInto(&n)
	if err := p.ParseArgs([]string{"prog", "--n", "+1"}); err == nil {
		t.Fatalf("expected error for leading '+'")
	}
}

func TestArgument_StoreInto_IntTrailingJunkDoesNotMatchToEnd(t *testing.T) {
	p := NewArgumentParser("prog")
	var n int
	p.AddArgument("--n").StoreInto(&n)
	err := p.ParseArgs([]string{"prog", "--n", "12x"})
	if err == nil {
		t.Fatalf("expected error")
	}
	if !strings.Contains(err.Error(), "does not match to the end") {
		t.Fatalf("expected 'does not match to the end' error, got: %v", err)
	}
}

func TestArgument_StoreInto_FloatRejectsLeadingPlus(t *testing.T) {
	p := NewArgumentParser("prog")
	var f float64
	p.AddArgument("--f").StoreInto(&f)
	if err := p.ParseArgs([]string{"prog", "--f", "+1"}); err == nil {
		t.Fatalf("expected error for leading '+'")
	}
}

func TestArgument_StoreInto_FloatTrailingJunkDoesNotMatchToEnd(t *testing.T) {
	p := NewArgumentParser("prog")
	var f float64
	p.AddArgument("--f").StoreInto(&f)
	err := p.ParseArgs([]string{"prog", "--f", "1.2x"})
	if err == nil {
		t.Fatalf("expected error")
	}
	if !strings.Contains(err.Error(), "Failed to parse '1.2x' as number") {
		t.Fatalf("unexpected error: %v", err)
	}
	if !strings.Contains(err.Error(), "does not match to the end") {
		t.Fatalf("unexpected error: %v", err)
	}
}

func TestArgument_Scan_RejectsLeadingPlus(t *testing.T) {
	{
		p := NewArgumentParser("prog")
		p.AddArgument("--n").Scan('d')
		if err := p.ParseArgs([]string{"prog", "--n", "+1"}); err == nil {
			t.Fatalf("expected error for leading '+' with scan('d')")
		}
	}

	{
		p := NewArgumentParser("prog")
		p.AddArgument("--f").Scan('g')
		if err := p.ParseArgs([]string{"prog", "--f", "+1"}); err == nil {
			t.Fatalf("expected error for leading '+' with scan('g')")
		}
	}
}

func TestArgument_Scan_FloatTrailingJunkDoesNotMatchToEnd(t *testing.T) {
	p := NewArgumentParser("prog")
	p.AddArgument("--f").Scan('g')
	err := p.ParseArgs([]string{"prog", "--f", "1.2x"})
	if err == nil {
		t.Fatalf("expected error")
	}
	if !strings.Contains(err.Error(), "Failed to parse '1.2x' as number") {
		t.Fatalf("unexpected error: %v", err)
	}
	if !strings.Contains(err.Error(), "does not match to the end") {
		t.Fatalf("unexpected error: %v", err)
	}
}

func TestArgumentParser_ChoicesInt(t *testing.T) {
	p := NewArgumentParser("prog")
	var n int
	p.AddArgument("--n").ChoicesInt(1, 2, 10).StoreInto(&n)

	if err := p.ParseArgs([]string{"prog", "--n", "2"}); err != nil {
		t.Fatalf("ParseArgs failed: %v", err)
	}
	if n != 2 {
		t.Fatalf("expected n=2, got %d", n)
	}

	if err := p.ParseArgs([]string{"prog", "--n", "3"}); err == nil {
		t.Fatalf("expected error for invalid choice")
	}
}

func TestArgumentParser_ChoicesInt_DefaultValidated(t *testing.T) {
	p := NewArgumentParser("prog")
	// Default is not in choices -> should fail validate during ParseArgs.
	p.AddArgument("--n").ChoicesInt(1, 2).DefaultValue(3)
	if err := p.ParseArgs([]string{"prog"}); err == nil {
		t.Fatalf("expected error for invalid default value not in choices")
	}
}

func TestArgumentParser_NargsAnyDoesNotOverflow(t *testing.T) {
	program := NewArgumentParser("p")
	vals := []string{}
	program.AddArgument("--vals").NargsPattern(NArgsAny).StoreInto(&vals)
	var x bool
	program.AddArgument("--x").StoreInto(&x).Nargs(0)

	// Should consume only positionals until next option.
	if err := program.ParseArgs([]string{"p", "--vals", "a", "b", "--x"}); err != nil {
		t.Fatalf("ParseArgs failed: %v", err)
	}
	if len(vals) != 2 || vals[0] != "a" || vals[1] != "b" {
		t.Fatalf("unexpected vals: %#v", vals)
	}
	if !x {
		t.Fatalf("expected x=true")
	}
}

func TestArgumentParser_AssignStyleLongOption(t *testing.T) {
	program := NewArgumentParser("p")
	var name string
	program.AddArgument("--name").StoreInto(&name)
	if err := program.ParseArgs([]string{"p", "--name=admin"}); err != nil {
		t.Fatalf("ParseArgs failed: %v", err)
	}
	if name != "admin" {
		t.Fatalf("expected name=admin, got %q", name)
	}
}

func TestArgument_StoreInto_Uint8RangeError(t *testing.T) {
	p := NewArgumentParser("p")
	var u uint8
	p.AddArgument("--u").StoreInto(&u)

	if err := p.ParseArgs([]string{"p", "--u", "255"}); err != nil {
		t.Fatalf("ParseArgs failed: %v", err)
	}
	if u != 255 {
		t.Fatalf("expected u=255, got %d", u)
	}

	if err := p.ParseArgs([]string{"p", "--u", "256"}); err == nil {
		t.Fatalf("expected error for uint8 overflow")
	}
}

func TestArgument_StoreInto_UintTrailingJunkDoesNotMatchToEnd(t *testing.T) {
	p := NewArgumentParser("prog")
	var u uint
	p.AddArgument("--u").StoreInto(&u)
	err := p.ParseArgs([]string{"prog", "--u", "12x"})
	if err == nil {
		t.Fatalf("expected error")
	}
	if !strings.Contains(err.Error(), "does not match to the end") {
		t.Fatalf("expected 'does not match to the end' error, got: %v", err)
	}
}

func TestArgument_StoreInto_Int8RangeError(t *testing.T) {
	p := NewArgumentParser("p")
	var n int8
	p.AddArgument("--n").StoreInto(&n)

	if err := p.ParseArgs([]string{"p", "--n", "127"}); err != nil {
		t.Fatalf("ParseArgs failed: %v", err)
	}
	if n != 127 {
		t.Fatalf("expected n=127, got %d", n)
	}

	if err := p.ParseArgs([]string{"p", "--n", "128"}); err == nil {
		t.Fatalf("expected error for int8 overflow")
	}
	if err := p.ParseArgs([]string{"p", "--n", "-129"}); err == nil {
		t.Fatalf("expected error for int8 underflow")
	}
}

func TestArgument_StoreInto_Float32RangeError(t *testing.T) {
	p := NewArgumentParser("p")
	var f float32
	p.AddArgument("--f").StoreInto(&f)

	if err := p.ParseArgs([]string{"p", "--f", "3.5"}); err != nil {
		t.Fatalf("ParseArgs failed: %v", err)
	}
	if f != float32(3.5) {
		t.Fatalf("expected f=3.5, got %v", f)
	}

	// Out of float32 range.
	if err := p.ParseArgs([]string{"p", "--f", "1e50"}); err == nil {
		t.Fatalf("expected error for float32 overflow")
	}
}

func TestArgumentParser_CompoundShortOptions(t *testing.T) {
	program := NewArgumentParser("p")
	var a, b, c bool
	program.AddArgument("-a").StoreInto(&a).Nargs(0)
	program.AddArgument("-b").StoreInto(&b).Nargs(0)
	program.AddArgument("-c").StoreInto(&c).Nargs(0)
	if err := program.ParseArgs([]string{"p", "-abc"}); err != nil {
		t.Fatalf("ParseArgs failed: %v", err)
	}
	if !a || !b || !c {
		t.Fatalf("expected a,b,c true; got %v %v %v", a, b, c)
	}
}

func TestArgumentParser_NegativeNumberIsPositional(t *testing.T) {
	program := NewArgumentParser("p")
	var x int
	program.AddArgument("x").StoreInto(&x)
	if err := program.ParseArgs([]string{"p", "-1"}); err != nil {
		t.Fatalf("ParseArgs failed: %v", err)
	}
	if x != -1 {
		t.Fatalf("expected x=-1, got %d", x)
	}
}

func TestArgumentParser_StopsValueConsumptionAtNextOption(t *testing.T) {
	program := NewArgumentParser("p")
	var name string
	program.AddArgument("--name").StoreInto(&name)
	program.AddArgument("--verbose").Nargs(0)
	err := program.ParseArgs([]string{"p", "--name", "--verbose"})
	if err == nil {
		t.Fatalf("expected error")
	}
}

func TestArgumentParser_ParseKnownArgsCollectsUnknown(t *testing.T) {
	program := NewArgumentParser("p")
	var name string
	program.AddArgument("--name").StoreInto(&name)
	unknown, err := program.ParseKnownArgs([]string{"p", "--unknown", "--name", "x", "--other"})
	if err != nil {
		t.Fatalf("ParseKnownArgs failed: %v", err)
	}
	if name != "x" {
		t.Fatalf("expected name=x, got %q", name)
	}
	if len(unknown) != 2 || unknown[0] != "--unknown" || unknown[1] != "--other" {
		t.Fatalf("unexpected unknown: %#v", unknown)
	}
}

func TestArgumentParser_RequiredArgument(t *testing.T) {
	program := NewArgumentParser("p")
	program.AddArgument("--name").Required()
	err := program.ParseArgs([]string{"p"})
	if err == nil {
		t.Fatalf("expected required error")
	}
}

func TestArgumentParser_DuplicateArgumentErrorsUnlessAppend(t *testing.T) {
	program := NewArgumentParser("p")
	var name string
	program.AddArgument("--name").StoreInto(&name)
	err := program.ParseArgs([]string{"p", "--name", "a", "--name", "b"})
	if err == nil {
		t.Fatalf("expected duplicate argument error")
	}
}

func TestArgumentParser_MutexGroup(t *testing.T) {
	program := NewArgumentParser("p")
	g := program.AddMutuallyExclusiveGroup(false)
	var a, b bool
	g.AddArgument("-a").StoreInto(&a).Nargs(0)
	g.AddArgument("-b").StoreInto(&b).Nargs(0)
	err := program.ParseArgs([]string{"p", "-a", "-b"})
	if err == nil {
		t.Fatalf("expected mutex violation")
	}
}

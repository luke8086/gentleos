#
# Copyright (c) 2026 luke8086
# Distributed under the terms of GPL-2 License.
#
# File: genmake.pl - Script for genereating Makefile and linker scripts
#

require "./tools/common.pl";

my @KERNEL_SOURCE_DIRS = ("apps", "data", "kernel", "lib", "gui");
my @BOOT2_SOURCE_DIRS = ("boot2");

my $MAKEFILE_TPL = <<'EOT';
all: disks .SYMBOLIC
    @echo All done!

disks: GT16.COM GT16.DAT build/boot1/boot1.bin build/boot2/boot2.com .SYMBOLIC
    perl tools/mkdisks.pl

run: GT16.COM GT16.DAT .SYMBOLIC
    GT16.COM

boot: all .SYMBOLIC
    boot GT16FD14.IMG

clean: .SYMBOLIC
    perl tools/clean.pl

KERNEL_OBJS = &
<KERNEL_OBJS>

BOOT2_OBJS = &
<BOOT2_OBJS>

INCLUDES = &
<INCLUDES>

INITRD_ASSETS = &
<INITRD_ASSETS>

build/boot1/boot1.bin: boot1/boot1.s
    nasm -o build/boot1/boot1.bin boot1/boot1.s

GT16.COM: $(KERNEL_OBJS)
	wlink @build/kernel.lnk

GT16.DAT: tools/mkdata.pl tools/common.pl $(INITRD_ASSETS)
    perl tools/mkdata.pl

build\boot2\boot2.com: $(BOOT2_OBJS)
    wlink @boot2/boot2.lnk

<OBJECT_RULES>
EOT

my $C_RULE_TPL = <<'EOT';
<OBJ_PATH>: <SRC> $(INCLUDES)
!ifdef TC
	tcc -mt -u -g1 -c -Iinclude -n<OBJ_DIR> <SRC>
!else
	wcc -fo=<OBJ_PATH> <SRC> @misc\wcc.occ
!endif
EOT

my $S_RULE_TPL = <<'EOT';
<OBJ_PATH>: <SRC>
	nasm -f obj -o <OBJ_PATH> <SRC>
EOT

sub path_to_dos {
    my ($p) = @_;
    $p =~ s{/}{\\}g;
    return $p;
}

sub dirname {
    my ($p) = @_;
    if ($p =~ m{^(.*)/[^/]*$}) { return $1; }
    return "";
}

sub splitext {
    my ($p) = @_;
    if ($p =~ m{^(.*)(\.[^./\\]*)$}) { return ($1, $2); }
    return ($p, "");
}

sub collect_kernel_sources {
    my @sources;

    foreach my $d (@KERNEL_SOURCE_DIRS) {
        push @sources, glob("$d/*.[cC]");
        push @sources, glob("$d/*.[sS]");
    }

    @sources = map(lc, @sources);
    @sources = sort @sources;
    @sources = grep { $_ ne "kernel/start.s" } @sources;
    unshift @sources, "kernel/start.s";

    return @sources;
}

sub collect_boot2_sources {
    my @sources;

    foreach my $d (@BOOT2_SOURCE_DIRS) {
        push @sources, glob("$d/*.[cC]");
        push @sources, glob("$d/*.[sS]");
    }

    @sources = map(lc, @sources);
    @sources = sort @sources;
    @sources = grep { $_ ne "boot2/start.s" } @sources;
    unshift @sources, "boot2/start.s";

    return @sources;
}

sub collect_includes {
    my @includes = sort(glob("include/*.h"));
    return @includes;
}

sub collect_initrd_assets {
    return sort((
        glob("assets/spk/*.spk"),
        glob("usrmedia/*.spk"),
        glob("vendor/misc/*.pbm"),
        glob("usrmedia/*.pbm"),
    ));
}

sub make_object_rule {
    my ($src) = @_;
    my ($base, $ext) = splitext($src);
    my $tpl = ($ext eq ".c") ? $C_RULE_TPL : $S_RULE_TPL;
    $tpl =~ s/\n+$//;

    my $obj_path = "build\\" . path_to_dos($base) . ".obj";
    my $obj_dir  = "build\\" . path_to_dos(dirname($src));
    my $src_dos  = path_to_dos($src);

    $tpl =~ s/<SRC>/$src_dos/g;
    $tpl =~ s/<OBJ_PATH>/$obj_path/g;
    $tpl =~ s/<OBJ_DIR>/$obj_dir/g;

    return $tpl;
}

sub make_obj_lines {
    my ($sources_ref) = @_;

    my $objs = join("\n", map {
        my ($b, $e) = splitext($_);
        "\tbuild/" . $b . ".obj &"
    } @$sources_ref);

    return $objs;
}

sub generate_makefile {
    my ($kernel_sources_ref, $boot2_sources_ref, $includes_ref, $initrd_assets_ref) = @_;

    my @all_sources = (@$kernel_sources_ref, @$boot2_sources_ref);
    my $kernel_objs = make_obj_lines($kernel_sources_ref);
    my $boot2_objs = make_obj_lines($boot2_sources_ref);
    my $incs = join("\n", map { "\t" . $_ . " &" } @$includes_ref);
    my $initrd_assets = join("\n", map { "\t" . $_ . " &" } @$initrd_assets_ref);
    my $rules = join("\n\n", map { make_object_rule($_) } @all_sources);

    my $content = $MAKEFILE_TPL;
    $content =~ s/<KERNEL_OBJS>/$kernel_objs/;
    $content =~ s/<BOOT2_OBJS>/$boot2_objs/;
    $content =~ s/<INCLUDES>/$incs/;
    $content =~ s/<INITRD_ASSETS>/$initrd_assets/;
    $content =~ s/<OBJECT_RULES>/$rules/;

    $content =~ s/\n/\r\n/g;

    update_file("Makefile", $content);
}

sub generate_kernel_lnk {
    my ($sources_ref) = @_;

    my @lines = (
        "system dos com",
        "name GT16.COM",
        "option nodefaultlibs",
        "option quiet",
        "option map=build/kernel.map",
    );

    foreach my $src (@$sources_ref) {
        my ($base, $ext) = splitext($src);
        my $obj_path = "build/" . $base . ".obj";
        push @lines, "file $obj_path";
    }

    push @lines, "";

    update_file("build/kernel.lnk", join("\r\n", @lines));
}

sub main {
    my @kernel_sources = collect_kernel_sources();
    my @boot2_sources = collect_boot2_sources();
    my @includes = collect_includes();
    my @initrd_assets = collect_initrd_assets();
    generate_makefile(\@kernel_sources, \@boot2_sources, \@includes, \@initrd_assets);
    generate_kernel_lnk(\@kernel_sources);
}

main();

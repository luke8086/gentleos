#
# Copyright (c) 2026 luke8086
# Distributed under the terms of GPL-2 License.
#
# File: genmake.pl - Script for genereating Makefile and linker scripts
#

my @KERNEL_SOURCE_DIRS = ("apps", "data", "kernel", "lib", "gui");
my @BOOT2_SOURCE_DIRS = ("boot2");

my $MAKEFILE_TPL = <<'EOT';
all: disks .SYMBOLIC
    @echo All done!

disks: build/gentleos.com build/boot1/boot1.bin build/boot2/boot2.com .SYMBOLIC
    perl tools/mkdisks.pl

run: all .SYMBOLIC
    build\gentleos.com

boot: all .SYMBOLIC
    boot build\fd1440.img

clean: .SYMBOLIC
    perl tools/clean.pl

KERNEL_OBJS = &
<KERNEL_OBJS>

BOOT2_OBJS = &
<BOOT2_OBJS>

INCLUDES = &
<INCLUDES>

build/boot1/boot1.bin: boot1/boot1.s
    nasm -o build/boot1/boot1.bin boot1/boot1.s

build\gentleos.com: $(KERNEL_OBJS)
	wlink @build/kernel.lnk

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
    my @includes = sort(glob("config.h include/*.h"));
    @includes = grep { !/_config\.h$/ } @includes;
    return @includes;
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
    my ($kernel_sources_ref, $boot2_sources_ref, $includes_ref) = @_;

    my @all_sources = (@$kernel_sources_ref, @$boot2_sources_ref);
    my $kernel_objs = make_obj_lines($kernel_sources_ref);
    my $boot2_objs = make_obj_lines($boot2_sources_ref);
    my $incs = join("\n", map { "\t" . $_ . " &" } @$includes_ref);
    my $rules = join("\n\n", map { make_object_rule($_) } @all_sources);

    my $content = $MAKEFILE_TPL;
    $content =~ s/<KERNEL_OBJS>/$kernel_objs/;
    $content =~ s/<BOOT2_OBJS>/$boot2_objs/;
    $content =~ s/<INCLUDES>/$incs/;
    $content =~ s/<OBJECT_RULES>/$rules/;

    $content =~ s/\n/\r\n/g;

    open(my $fh, ">", "Makefile") or die "Cannot open Makefile: $!";
    binmode($fh);
    print $fh $content;
    close($fh);
}

sub generate_kernel_lnk {
    my ($sources_ref) = @_;

    my @lines = (
        "system dos com",
        "name build/gentleos.com",
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

    open(my $fh, ">", "build/kernel.lnk") or die "Cannot open build/kernel.lnk: $!";
    binmode($fh);
    print $fh join("\r\n", @lines);
    close($fh);
}

sub main {
    my @kernel_sources = collect_kernel_sources();
    my @boot2_sources = collect_boot2_sources();
    my @includes = collect_includes();
    generate_makefile(\@kernel_sources, \@boot2_sources, \@includes);
    generate_kernel_lnk(\@kernel_sources);
}

main();

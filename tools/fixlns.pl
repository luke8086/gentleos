#
# Copyright (c) 2026 luke8086
# Distributed under the terms of GPL-2 License.
#
# File: fixlns.pl - Fix line endings in source code
#

require "./tools/common.pl";

my @GLOBS = (
    "apps/*",
    "boot1/*",
    "boot2/*",
    "data/*",
    "gui/*",
    "include/*",
    "kernel/*",
    "lib/*",
    "misc/*",
    "tools/*.pl",
);

sub collect_files {
    my @files;
    foreach my $g (@GLOBS) {
        push @files, grep { -f $_ } glob($g);
    }
    return sort @files;
}

sub main {
    foreach my $path (collect_files()) {
        my $old = slurp($path);
        my $new = $old;
        $new =~ s/\r\n/\n/g;
        $new =~ s/\n/\r\n/g;

        next if $new eq $old;

        print "Fixing $path\n";
        spit($path, $new)
    }
}

main();

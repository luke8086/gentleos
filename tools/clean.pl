#
# Copyright (c) 2026 luke8086
# Distributed under the terms of GPL-2 License.
#
# File: clean.pl - Script for cleaning up build artifacts
#

use File::Path;

rmtree('build');
unlink('data/data.c');
unlink('Makefile');
unlink('GT16.COM');
unlink('GT16.DAT');
unlink("GT16DISK.IMG");
unlink("GT16FD72.IMG");
unlink("GT16FD14.IMG");
unlink("GT16WEB.IMG");

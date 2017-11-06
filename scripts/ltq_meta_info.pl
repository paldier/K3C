#!/usr/bin/env perl
#
# This script dumps the basic package information as csv file.
#
# ltq_meta_info.pl > package_info.csv
#

use strict;
use warnings;

my $PWD=`pwd`;
my @search_path = ( "./package", "./package/feeds/packages" );

sub dump_info {

	my $path = shift;

	open(F, "ls $path | ");
	while (<F>) {
		chomp();
		my $line = $_;
		if ( -d "$path/$line" ) {
			my $package="";
			my $version ="";
			my $title="";
			my $description="";
			my $url="";
			#print "\n### make -s -C $path/$line DUMP=1 TOPDIR=$PWD";
			open(M, "make -s -C $path/$line DUMP=1 TOPDIR=$PWD 2> /dev/null | ");
			while (<M>) {
				chomp();
				my $info = $_;
				if( $info =~ 'Package:') {
					$info=~ s/Package: //;
					$package = $info;
				}
				if( $info =~ 'Version:') {
					$info=~ s/Version: //;
					$version = $info;
				}
				if( $info =~ 'Title:') {
					$info=~ s/Title: //;
					$title = $info;
				}
				if( $info =~ 'Description:') {
					$info=~ s/Description: //;
					$info=~ s/^(\ )*//;
					$description = $info;
					while (<M>) {
						chomp();
						$info = $_;
						$info=~ s/^(\ )*//;
						last if( $info =~ '^http:\/\/');
						last if( $info =~ '@@');
						$description = $description . '\n' . $info;
					}
				}
				if( $info =~ '^http:\/\/') {
					$url = $info;
				}
			}
			if( $package ne "" and $version ne "" and $title ne "") {
				print("$package;$version;$title;$url;$description\n");
				$package="";
				$version ="";
				$title="";
				$url="";
			}
			close(M);
		}
	}
	close(F);
}

dump_info($search_path[0]);
dump_info($search_path[1]);

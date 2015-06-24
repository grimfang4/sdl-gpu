#!/usr/bin/perl -w

##################################################################
# 
# Convenince script to invoke make on CMake projects for Android 
# for each architecture and build them.
# Copyright (C) PlayControl Software, LLC. 
# Eric Wing <ewing . public @ playcontrol.net>
#
##################################################################


use strict;
use warnings;

# Function to help with command line switches
use Getopt::Long;
# Allows extra "unknown options" to be specified which I will use to pass directly to the cmake executable.
Getopt::Long::Configure("pass_through");

# Function to get the basename of a file
use File::Basename;
# Used for tilde expansion
use File::Glob;

# Provides functions to convert relative paths to absolute paths.
use Cwd;

# Global constants
my %kArchToDirectoryNameMap =
(
#	mips => "mips",
	armeabi => "armeabi",
	"armeabi-v7a" => "armeabi-v7a",
#	"armeabi-v7a with NEON" => "armeabi-v7a",
#	"armeabi-v7a with VFPV3" => "armeabi-v7a",
#	"armeabi-v6 with VFP" => "armeabi",
	x86 => "x86"
);

# Global constants
my %kArchToCompilerNameMap =
(
#	mips => "mips",
	armeabi => "arm-linux-androideabi",
	"armeabi-v7a" => "arm-linux-androideabi",
#	"armeabi-v7a with NEON" => "armeabi",
#	"armeabi-v7a with VFPV3" => "armeabi",
#	"armeabi-v6 with VFP" => "armeabi",
	x86 => "x86"
);


my @kSupportedArchitectures =
(
#	"mips",
	"armeabi",
	"armeabi-v7a",
#	"armeabi-v7a with NEON",
#	"armeabi-v7a with VFPV3",
#	"armeabi-v6 with VFP",
	"x86",
);


# Function prototypes 

# main routine
sub main();
# call main
main();

sub main()
{
	my ($targetdir, $make, @remaining_options) = extract_parameters();
	# Save in case we need to return to the original current working directory.	
#	my $original_current_working_directory = Cwd::cwd();

#	print("targetdir: ", $targetdir, "\n"); 
#	print("make: ", $make, "\n"); 
#	print("remaining_options: ", @remaining_options, "\n"); 


	foreach my $arch(@kSupportedArchitectures)
	{
		chdir($targetdir) or die("Could not change directory to $targetdir: $!\n");
		my $arch_dir = $kArchToDirectoryNameMap{$arch};
		print("Building $arch\n");
		chdir($arch_dir) or die("Could not change directory to $arch_dir: $!\n");

		my $error_status = system($make, @remaining_options);
		if($error_status != 0)
		{
			die "Invoking make failed: $?\n";
		}

	}

	return;

}


sub helpmenu()
{
	my $basename = basename($0);
	print "Convenience script for invoking make on CMake based projects for Android (multiple architectures).\n\n";

	print "Usage: perl $basename [-h | -help] --targetdir=<path to build dir> [--make=<Make exectuable>] [<other flags passed to Make>]\n";

	print "Options:\n";
	print "  -h or -help                              Brings up this help display.\n";
	print "  --targetdir=<path to build directory>    (Optional) Path to where the root of the build directory is. Default is the current working directory.\n";
	print "  --make=<Make executable>                 (Optional) Allows you to specify the path and file to the Make executable.\n";
	print "\n";
	print "Example Usage:\n";
	print "$basename clean\n";
	print "$basename VERBOSE=1\n";

	return;
}

sub home_dir() 
{
	return File::Glob::bsd_glob("~");
}

sub expand_tilde($)
{
	my $path = shift;
	my $home_dir = home_dir();

	$path =~ s/^~/$home_dir/;
	return $path;
}

sub absolute_path($)
{
	my $file = shift;
	return Cwd::abs_path(expand_tilde($file));
}

# Subroutine to extract and process command line parameters
sub extract_parameters()
{
	my %params = (
		h => \(my $hflag = 0),
		help => \(my $helpflag = 0),
		targetdir => \(my $targetdir),
		make => \(my $make)
   );

	# Call Library function which will extract and remove all switches and
	# their corresponding values.
	# These parameters will be removed from @ARGV
	my $errorval = &GetOptions(\%params, "h", "help",
					"targetdir=s",
					"make=s"
	); 
	# the exclaimation point allows for the negation
	# of the switch (i.e. -nobackup/-nobody is a switch)


	# Error value should have returned 1 if nothing went wrong
	# Otherwise, an unlisted parameter was specified.
	if($errorval !=1)
	{
		# Expecting GetOptions to state error.

		print "Exiting Program...\n";
		exit 0;
	}

	if( ($hflag == 1) || ($helpflag == 1) ) 
	{
		helpmenu();
		exit 0;
	}

	if(not defined($targetdir))
	{
		$targetdir = Cwd::cwd();
	}

	if(not defined($make))
	{
		$make = "make";
	}
	else
	{
		$make = absolute_path($make);	
	}
	$targetdir = absolute_path($targetdir);

	my @remaining_options = @ARGV;
	my @sorted_options = ($targetdir, $make, @remaining_options);
	
	return @sorted_options;
}



#!/usr/bin/env perl -w

###########################################################
# 
# Convenince script to generate CMake projects for Android 
# for each architecture and build them.
# Copyright (C) PlayControl Software, LLC. 
# Eric Wing <ewing . public @ playcontrol.net>
#
# Convention:
# You have created standalone toolchains and placed them in a directory called standalone under the $ANDROID_NDK_ROOT:
# $ANDROID_NDK_ROOT/standalone/
#	arm-linux-androideabi-4.6/	
# 	x86-4.6/
# 	
##########################################################


#use strict;
use warnings;

# Function to help with command line switches
use Getopt::Long;
# Allows extra "unknown options" to be specified which I will use to pass directly to the cmake executable.
Getopt::Long::Configure("pass_through");

# Function to get the basename of a file
use File::Basename;
# Used for tilde expansion
use File::Glob;
# for make_path (which is mkdir -p)
use File::Path qw(make_path);

# Provides functions to convert relative paths to absolute paths.
use Cwd;

use File::Basename;

# Global constants

my $kCMakeBootStrapCacheFile = "InitialCache_Android.cmake";

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

sub main()
{

	my ($blurrr_sdk_root,
		$blurrr_sdk_path, 
		$cmake_source_dir, 
		$cmake_binary_dir, 
		$libsdir, 
		$android_sdk_root, 
		$standalone_toolchain_root, 
		$targetSdkVersion,
		$minSdkVersion,
		$compilerversion, 
		$buildtype,
		$should_build, 
		$cmake_toolchain,
		$cmake,
		$architectures,
		@remaining_options
	) = extract_parameters();

	my @architectures_array;

	if(not defined($architectures))
	{
		@architectures_array = @kSupportedArchitectures;
	}
	else
	{
		@architectures_array = split(/[ ,;]/, $architectures);
	}




	print("blurrr_sdk_root: ", $blurrr_sdk_root, "\n"); 
	print("blurrr_sdk_path: ", $blurrr_sdk_path, "\n"); 
	print("cmake_source_dir: ", $cmake_source_dir, "\n"); 
	print("cmake_binary_dir: ", $cmake_binary_dir, "\n"); 
	print("libsdir: ", $libsdir, "\n"); 


	print("android_sdk_root: ", $android_sdk_root, "\n"); 
	print("standalone_toolchain_root: ", $standalone_toolchain_root, "\n"); 
	print("targetSdkVersion: ", $targetSdkVersion, "\n"); 
	print("minSdkVersion: ", $minSdkVersion, "\n"); 
#	print("compilerversion: ", $compilerversion, "\n"); 
	print("buildtype: ", $buildtype, "\n"); 
	print("should_build: ", $should_build, "\n"); 
	print("cmake_toolchain: ", $cmake_toolchain, "\n"); 
	print("cmake: ", $cmake, "\n"); 
	
#	print("remaining_options: ", @remaining_options, "\n"); 



	copy_build_scripts($cmake_source_dir, $cmake_binary_dir);



#	my ($targetdir, $standalone, $compilerversion, $should_build, $cmake, $toolchain, $libsdir, $buildtype, $sourcedir, @remaining_options) = extract_parameters();

	# Save in case we need to return to the original current working directory.
	my $original_current_working_directory = Cwd::cwd();

	foreach my $arch(@architectures_array)
	{
		# First choose the correct compiler.
		my $found_compiler;
		my $compiler_base_name = $kArchToCompilerNameMap{$arch};
		
		opendir(STANDALONEDIR, $standalone_toolchain_root) or die("Could not open standalone_toolchain_root directory: $!\n");
    	while(my $file = readdir(STANDALONEDIR))
		{
			my $full_path_and_file = "$standalone_toolchain_root/$file";

			# Go to the next file unless it is a directory
			next unless(-d "$full_path_and_file");

			# if a version was specified, make sure it matches
			if(defined($compilerversion))
			{
				if($file =~ m/$compiler_base_name-$compilerversion/)
				{
					$found_compiler = $full_path_and_file;
					last;
				}
			}
			# otherwise if no version was specified, just go for any match
			else
			{
				if($file =~ m/$compiler_base_name/)
				{
					$found_compiler = $full_path_and_file;
					last;
				}
			}
		}
		closedir(STANDALONEDIR);

		if(not defined $found_compiler)
		{
			die("Could not find compiler in directory:$standalone_toolchain_root for arch:$arch\n");
		}



		chdir($cmake_binary_dir) or die("Could not change directory to $cmake_binary_dir: $!\n");
		# Let's make an intermediate subdirectory for all the CMake NDK stuff we're going to create
		my $cmake_ndk_build = "cmake_ndk_build";
 		unless(-e $cmake_ndk_build or mkdir $cmake_ndk_build)
		{
			die("Unable to create $cmake_ndk_build: $!\n");
		}
		chdir($cmake_ndk_build) or die("Could not change directory to $cmake_ndk_build: $!\n");
		
		my $arch_dir = $kArchToDirectoryNameMap{$arch};
		unless(-e $arch_dir or mkdir $arch_dir)
		{
			die("Unable to create $arch_dir: $!\n");
		}
		chdir($arch_dir) or die("Could not change directory to $arch_dir: $!\n");

		my $android_standalone_toolchain = "$found_compiler";
		print("Generating $arch\n");

		#my $initial_cache = "$blurrr_sdk_root/$kCMakeBootStrapCacheFile";
		my $initial_cache = "$cmake_source_dir/AndroidCMake/$kCMakeBootStrapCacheFile";
		my $suppress_dev_warnings_flags = "-Wno-dev";

		my $blurrr_path_to_use;
		if(defined $blurrr_sdk_path)
		{
			$blurrr_path_to_use = "-DBLURRR_SDK_PATH=$blurrr_sdk_path";
		}
		else
		{
			$blurrr_path_to_use = "-DBLURRR_ROOT=$blurrr_sdk_root";
		}

		# There is something screwing up the call when I do comma separated arguments.
		my $command_string = "$cmake $suppress_dev_warnings_flags -DCMAKE_TOOLCHAIN_FILE=$cmake_toolchain $blurrr_path_to_use -DBLURRR_SDK_PATH=$blurrr_sdk_path -DANDROID_ABI=$arch -DBLURRR_CMAKE_ANDROID_REAL_BINARY_DIR=$cmake_binary_dir -DANDROID_SDK_ROOT=$android_sdk_root -C $initial_cache -DANDROID_STANDALONE_TOOLCHAIN=$android_standalone_toolchain -DANDROID_TARGET_SDK_VERSION=$targetSdkVersion -DANDROID_MIN_SDK_VERSION=$minSdkVersion -DLIBRARY_OUTPUT_PATH_ROOT=$libsdir -DCMAKE_BUILD_TYPE=$buildtype @remaining_options $cmake_source_dir";
		#print("Executing: $cmake $cmake_toolchain $blurrr_root_flag $arch_flag $initial_cache $android_standalone_toolchain $libsdir $buildtype @remaining_options $cmake_source_dir\n");
		print("Executing: $command_string\n\n");

#
# /Volumes/DataPartition/Users/ewing/Source/Blurrr/Templates/DIST/CMake/CMake.app/Contents/bin/cmake -DCMAKE_TOOLCHAIN_FILE=/Volumes/DataPartition/Users/ewing/Source/Blurrr/Templates/DIST/Templates/C/CMakeModules/android.toolchain.cmake -DBLURRR_ROOT=/Volumes/DataPartition/Users/ewing/Source/Blurrr/Templates/DIST -DANDROID_ABI=armeabi -DBLURRR_CMAKE_ANDROID_REAL_BINARY_DIR=/Volumes/DataPartition/Users/ewing/TEMP/GradleCMakeTest -C /Volumes/DataPartition/Users/ewing/Source/Blurrr/Templates/DIST/bootstrap/InitialCache_C_Android.cmake -DANDROID_STANDALONE_TOOLCHAIN=/Library/Frameworks/Android/android-ndk-r9b/standalone/arm-linux-androideabi-4.6 -DLIBRARY_OUTPUT_PATH_ROOT=/Volumes/DataPartition/Users/ewing/TEMP/GradleCMakeTestapp/libs -DCMAKE_BUILD_TYPE=Release  /Volumes/DataPartition/Users/ewing/Source/Blurrr/Templates/DIST/Templates/Cloading initial cache file /Volumes/DataPartition/Users/ewing/Source/Blurrr/Templates/DIST/bootstrap/InitialCache_C_Android.cmake
		# Spaces in paths can really system() up, so the comma separated version is better because it is able to distinguish the difference between a space vs. multiple arguments.
		my $error_status = system($cmake, 
			$suppress_dev_warnings_flags, 
			"-DCMAKE_TOOLCHAIN_FILE=$cmake_toolchain", 
			$blurrr_path_to_use, 
			"-DBLURRR_SDK_PATH=$blurrr_sdk_path", 
			"-DANDROID_ABI=$arch", 
			"-DBLURRR_CMAKE_ANDROID_REAL_BINARY_DIR=$cmake_binary_dir",
			"-DANDROID_SDK_ROOT=$android_sdk_root",
			"-C", $initial_cache, 
			"-DANDROID_STANDALONE_TOOLCHAIN=$android_standalone_toolchain", 
			"-DANDROID_TARGET_SDK_VERSION=$targetSdkVersion", 
			"-DANDROID_MIN_SDK_VERSION=$minSdkVersion", 
			"-DLIBRARY_OUTPUT_PATH_ROOT=$libsdir",
			"-DCMAKE_BUILD_TYPE=$buildtype", 
			@remaining_options,
			$cmake_source_dir
		);
#		my $error_status = system($command_string);
		if($error_status != 0)
		{
			die "Invoking CMake failed: $?\n";
		}

		if($should_build)
		{
			print("Building $arch\n");
			$error_status = system("make");
			if($error_status != 0)
			{
				die "Invoking make failed: $?\n";
			}
		}
	}

	return;

}


sub helpmenu()
{
	my $basename = basename($0);
	print "Convenience script for generating and building CMake based projects for Android (multiple architectures).\n\n";
	print "Convention:\n";
	print "You have created standalone toolchains and placed them in a directory called standalone under the \$ANDROID_NDK_ROOT:\n";
	print "\$ANDROID_NDK_ROOT/standalone/\n";
	print "\tarm-linux-androideabi-4.6/\n";
	print "\tx86-4.6/\n";

	#print "Usage: perl $basename [-h | -help] --sourcedir=<path to source> --targetdir=<path to build dir> --toolchain=<CMake toolchain file> [--standalone=<standalone root directory>] [--cmake=<CMake exectuable>] [--buildtype=<None|Debug|Release*|RelWithDebInfo|MinSizeRel>] [<other flags passed to CMake>] <Project Source Directory>\n";
	print "Usage: perl $basename [-h | -help] [<other flags passed to CMake>] <Project Source Directory>\n";

	print "Options:\n";
	print "  -h or -help                             Brings up this help display.\n";
	print "\n";

	print "Required Options:\n";
	print "<Project Source Directory>                Path to the source code directory (containing the root CMakeLists.txt)\n";
	print "\n";

	print "Semi-required Options (needed if required environmental variables are not set)\n";
	print "  --androidsdkroot=<path to Android SDK>  Path to the Android SDK. The environmental variable ANDROID_SDK_ROOT or ANDROID_HOME is used if not provided.\n";
	print "  --standalonetoolchainroot=<path>        Path to the root of the NDK standalone tool chain you are required to generate. The environmental variable BLURRR_ANDROID_STANDALONE_ANDROID_TOOLCHAIN_ROOT is used if not provided.\n";
	print "  --blurrrsdkpath=<path>   			     Path to the SDK to use, e.g. ~/Blurrr/Libraries/Android/SDK/Lua_f32_i32. You may define this in an environmental variable. This acts as an overrride to BLURRR_ROOT.\n";
	print "\n";

	print "Optional Options:\n";
	print "  --targetsdkversion=<version>            Allows you to override the targetSdkVersion. Default detects the latest availabe in the SDK. (Best to use default.)\n";
	print "  --minsdkversion=<version>               Allows you to override the minSdkVersion. Default is 14. (Going lower may cause your app to crash on older devices.)\n";
	print "  --architectures=<list;of;arches>        Allows you to override which architectures to build. Default is \"armeabi;armeabi-v7a;x86\"\n";
	print "  --projectsourcedir=<path to source>     Overrides the <Project Source Directory> if you need a --switch.\n";
#	print "  --cmakebinarydir=<path to build dir>    Path to where the CMake projects will be generated. Current directory is assumed and is generally better to use than this.\n";
	print "  --cmaketoolchain=<toolchain file>       Path to and file of the CMake toolchain to use. Default finds the Android toolchain shipped in the Blurrr SDK.\n";
#	print "  --libsdir=<path where libs are copied>  Path where the built libs are placed. \n";
	print "  --compilerversion=<version>             Allows you to specify the version number (4.6) of the compiler to disambiguate if you have multiple versions.\n";
	print "  --cmake=<CMake executable>              Allows you to specify the path and file to the CMake executable. Default tries to find it in the Blurrr SDK or your environment.\n";
	print "  --buildtype=<build type>                The CMake Build Type. Default is MinSizeRel. None|Debug|Release|RelWithDebInfo|MinSizeRel\n";
	print "  --[no]build                             Specifies whether make should be invoked. Default is nobuild.\n";
	print "\n";
	print "Example Usage:\n";
	print "$basename ~/Source/Blurrr/Examples/Oblivion/OblivionC\n";

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

sub get_path_of_main_script()
{
	my $dirname = dirname(__FILE__);
	return absolute_path($dirname);
#	return $dirname;
}

sub copy_build_scripts($$)
{
	my $cmake_source_dir = shift;
	my $cmake_target_dir = shift;


	my $copy = "cp";
#	my $flags = "-f";
#	my $exclude_flags = ' --exclude ".in"';
	my $flags = '-f';
	my $source_param = $cmake_source_dir . "/AndroidCMake/*_ndk.sh";
	my $target_param = $cmake_target_dir;
	my $command = "$copy $flags $source_param $target_param";

	print("Executing: $copy $flags $source_param $target_param\n");
#	my $error_status = system($rsync, $flags, $exclude_flags, $source_param, $target_param);
	my $error_status = system($command);
	if($error_status != 0)
	{
		die "Invoking copy failed: $?\n";
	}	
}

sub sort_api_levels
{
	# $a and $b will be automatically defined by sort().
	# Look up the position of the day in the week
	# using the DAYS hash.
	my ($api_level_a) = $a =~ /android-(\d+)/;
	my ($api_level_b) = $b =~ /android-(\d+)/;
	
	# reverse sort
	return $api_level_b <=> $api_level_a;

	# Now we can just compare $x and $y. Note, it
	# would be simpler in this case to use the <=>
	# operator.
}

sub detect_highest_android_sdk_api_level($)
{
	my $sdk_root = shift;
	my $sdk_platforms = $sdk_root . "/platforms";
	
	opendir(DIR, $sdk_platforms);
	my @files = grep(/^android-\d+$/, readdir(DIR));
	closedir(DIR);

	# Sort days in order using a sort algorithm
	# contained in a function.
	@files = sort sort_api_levels @files;


#	foreach $file(@files)
#	{
#		print "$file\n";
#	}

	my ($api_level) = $files[0] =~ /android-(\d+)/;
	return $api_level;
}

# Subroutine to extract and process command line parameters
sub extract_parameters()
{
	# To follow the convention of the other bootstrap scripts and CMake itself,
	# the last parameter is always the path to the source directory.
	my $cmake_source_dir = $ARGV[$#ARGV];

	# Blurrr assumption: This Perl script is located at the root of the Blurrr SDK.
	my $blurrr_sdk_root = get_path_of_main_script();
	
	my %params = (
		h => \(my $hflag = 0),
		help => \(my $helpflag = 0),
		blurrrsdkpath => \(my $blurrr_sdk_path), # acts as an override for blurrr_sdk_root
		projectsourcedir => \(my $projectsourcedir), # this is allowed to override $cmake_source_dir
		cmakebinarydir => \(my $cmake_binary_dir), # 
		libsdir => \(my $libsdir),
		androidsdkroot => \(my $android_sdk_root),
		standalonetoolchainroot => \(my $standalone_toolchain_root),
		targetsdkversion => \(my $targetSdkVersion),
		minsdkversion => \(my $minSdkVersion = 14),
		architectures => \(my $architectures),
		compilerversion => \(my $compilerversion),
		buildtype => \(my $buildtype = "MinSizeRel"),
		build => \(my $should_build = 0),
		cmaketoolchain => \(my $cmake_toolchain),
		cmake => \(my $cmake)
       );


	# Call Library function which will extract and remove all switches and
	# their corresponding values.
	# These parameters will be removed from @ARGV
	my $errorval = &GetOptions(\%params, "h", "help",
					"blurrrsdkpath=s",
					"projectsourcedir=s",
					"cmakebinarydir=s",
					"libsdir=s",
					"androidsdkroot=s",
					"standalonetoolchainroot=s",
					"targetsdkversion=i",
					"minsdkversion=i",
					"architectures=s",
					"compilerversion=s",
					"buildtype=s",
					"build!",
					"cmaketoolchain=s",
					"cmake=s"
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

	if(not defined($projectsourcedir))
	{
		# $projectsourcedir = $cmake_source_dir;
	}
	else
	{
		$cmake_source_dir = $projectsourcedir;
	}
	# sanitize path
	$cmake_source_dir = absolute_path($cmake_source_dir);	


	if(not defined($cmake_binary_dir))
	{
		# This is the current working directory following CMake conventions.
		$cmake_binary_dir = absolute_path(getcwd());
	}
	# sanitize path
	$cmake_binary_dir = absolute_path($cmake_binary_dir);	



	if(not defined($libsdir))
	{
		$libsdir = $cmake_binary_dir . "/dist";
	}


	if(not defined($android_sdk_root))
	{
		$android_sdk_root = $ENV{"ANDROID_SDK_ROOT"} or $android_sdk_root = $ENV{"ANDROID_HOME"};
		if(not defined($android_sdk_root))
		{
			print("androidsdkroot directory not specified or environmental variable ANDROID_SDK_ROOT not defined\n");
			helpmenu();
			exit 1;
		}
	}
	# sanitize path
	$android_sdk_root = absolute_path($android_sdk_root);	

	if(not defined($blurrr_sdk_path))
	{
		$blurrr_sdk_path = $ENV{"BLURRR_SDK_PATH"};
		if(not defined($blurrr_sdk_path))
		{
			$blurrr_sdk_path = undef;
		}
	}
	else
	{
		$blurrr_sdk_path = absolute_path($blurrr_sdk_path);
	}


	if(not defined($standalone_toolchain_root))
	{
		$standalone_toolchain_root = $ENV{"BLURRR_ANDROID_STANDALONE_ANDROID_TOOLCHAIN_ROOT"};
		if(not defined($standalone_toolchain_root))
		{
			print("standalonetoolchainroot root directory not specified or environmental variable BLURRR_ANDROID_STANDALONE_ANDROID_TOOLCHAIN_ROOT not defined\n");
			helpmenu();
			exit 2;
		}
		
	}
	# sanitize path
	$standalone_toolchain_root = absolute_path($standalone_toolchain_root);	






	if(not defined($targetSdkVersion))
	{
		$targetSdkVersion = detect_highest_android_sdk_api_level($android_sdk_root);
	}

	# test $architectures on return because it is too hard to pass multiple arrays back

	if(not defined($cmake_toolchain))
	{
		# We keep a copy of the toolchain in the PROJECT_SOURCE_DIR/CMakeModules directory
		#$cmake_toolchain = $cmake_source_dir . "/CMakeModules/android.toolchain.cmake";
		$cmake_toolchain = $cmake_source_dir . "/AndroidCMake/android.toolchain.cmake";
	}

	if(not defined($cmake))
	{
		# Grrrr. On Mac & Windows, we ship CMake with the SDK, but on Linux we depend on package management.
		$cmake = $blurrr_sdk_root . "/CMake/CMake.app/Contents/bin/cmake";
		if(!(-f $cmake))
		{
			$cmake = $blurrr_sdk_root . "/CMake/bin/cmake.exe";
			if(!(-f $cmake))
			{
				# assume it is in the path and Unix-y (no .exe)
				$cmake = "cmake";
			}
		}
	}
	else
	{
		$cmake = absolute_path($cmake);	
	}


	# Convert to absolute paths because we will be changing directories which will break relative paths.

	# We need to create the directory if it doesn't exist before calling absolute_path() on it.
	unless(-e $libsdir or make_path($libsdir))
	{
		die("Unable to create $libsdir: $!\n");
	}


	$cmake_toolchain = absolute_path($cmake_toolchain);
	$libsdir = absolute_path($libsdir);


	# This can be optimized out, but is left for clarity. 
	# We already assumed the last parameter is the source dir and used it, so pop it.
#	pop @ARGV;

	# GetOptions has removed all found options so anything left in @ARGV is "remaining".
	my @remaining_options = @ARGV;
	pop @remaining_options;
	# Remember, can't pass back 2 arrays or in the middle because it shifts everything

	my @sorted_options = (
		$blurrr_sdk_root,
		$blurrr_sdk_path,
		$cmake_source_dir, 
		$cmake_binary_dir, 
		$libsdir, 
		$android_sdk_root, 
		$standalone_toolchain_root, 
		$targetSdkVersion,
		$minSdkVersion,
		$compilerversion, 
		$buildtype,
		$should_build, 
		$cmake_toolchain,
		$cmake,
		$architectures,
		@remaining_options
	);	
	return @sorted_options;
}


print("calling main\n");
main();



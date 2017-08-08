---
title: xcodebuild
date: 2017-03-03 10:17:52
tags:
- iOS
---
## xcodebuild

xcodebuild是一个命令行工具，可以用来对Xcode工程或工作区进行编译、查找、分析、测试等各种操作。对于如何使用xcode可以参看http://help.apple.com/xcode/mac/8.0/
<!-- more -->
### 基本概念

* workspace 是最大的集合，其可以包含多个 projet，同时可以管理不同的 project 之间的关系。workspace 是以 xcworkspace 的文件形式存在的。workspace 的存在是为了解决原来仅有 project 的时候不同的 project 之间的引用和调用困难的问题
* project是Xcode对一些代码文件、资源、配置信息的一个管理仓库，可以用来编译产出一个或多个产品。Project包含了多个targets，project有一个被所有targets公用的build setting。
* targets 表示如何产出一个具体产品，决定了如何对文件进行编译（编译那几个文件，使用什么编译脚本）（通过自己的build setting 和从project继承的build setting）
* configuration 可以对project和targets中
* scheme（方案）它不同于上面四个，表示一个组合方案表示那些targets使用什么 build configuration与 executable configuration 。An Xcode scheme defines a collection of targets to build, a configuration to use when building, and a collection of tests to execute.可以针对编译，运行，单元测试，动态分析，静态代码分析以及打包进行一些配置

### 命令参数

```shell
XCODEBUILD(1)             BSD 通用命令手册          XCODEBUILD(1)

命令名
     xcodebuild -- 可以用来编译 Xcode projects and workspaces

简介
     xcodebuild [-project projectname] [-target targetname ...] [-configuration configurationname]
                [-sdk [sdkfullpath | sdkname]] [buildaction ...] [setting=value ...]
                [-userdefault=value ...]
     xcodebuild [-project projectname] -scheme schemename [-destination destinationspecifier]
                [-destination-timeout value] [-configuration configurationname]
                [-sdk [sdkfullpath | sdkname]] [buildaction ...] [setting=value ...]
                [-userdefault=value ...]
     xcodebuild -workspace workspacename -scheme schemename [-destination destinationspecifier]
                [-destination-timeout value] [-configuration configurationname]
                [-sdk [sdkfullpath | sdkname]] [buildaction ...] [setting=value ...]
                [-userdefault=value ...]
     xcodebuild -version [-sdk [sdkfullpath | sdkname]] [infoitem]
     xcodebuild -showsdks
     xcodebuild -list [-project projectname | -workspace workspacename]
     xcodebuild -exportArchive -exportFormat format -archivePath xcarchivepath -exportPath destinationpath
                [-exportProvisioningProfile profilename] [-exportSigningIdentity identityname]
                [-exportInstallerIdentity identityname]

描述
     xcodebuild 构建Xcode project中的一个或多个targets, 也可以构建Xcode workspace或Xcode project的一个scheme。
     要构建一个Xcode project, 在你的工程目录下运行xcodebuild. 如果这个目录下有多个project文件则可以使用-project参数用来区分你要编译哪个project.  默认情况下,xcodebuild 构建project中列出的第一个target, 并且使用默认的build configuration.  targets在project中的顺序是project的一个属性对所有的使用者来说这个顺序是相同的。
     要构建一个Xcode workspace, 必须要使用参数-workspace 和 -scheme 指定构建。 scheme 控制了需要编译那些targets以及这个targets如何被编译。不过也可以通过 xcodebuild的其他参数来从新指定scheme的一些属性.

     其他的一些参数对编译工程没有作用，如下：
     -version, -showsdks, and -usage。可以用例显示一些编译工具信息。
     
参数
     -project projectname
           Build the project specified by projectname.  Required if there are multiple project files in the
           same directory.

     -target targetname
           Build the target specified by targetname.

     -alltargets
           Build all the targets in the specified project.

     -workspace workspacename
           Build the workspace specified by workspacename.

     -scheme schemename
           Build the scheme specified by schemename.  Required if building a workspace.

     -destination destinationspecifier
           Use the destination device described by destinationspecifier.  Defaults to a destination that is
           compatible with the selected scheme.  See the section on destinations for more details.

     -destination-timeout timeout
           Use the specified timeout when searching for a destination device. The default is 30 seconds.

     -configuration configurationname
           Use the build configuration specified by configurationname when building each target.

     -arch architecture
           Use the architecture specified by architecture when building each target.

     -sdk [<sdkfullpath> | <sdkname>]
           Build an Xcode project or workspace against the specified SDK, using build tools appropriate for
           that SDK. The argument may be an absolute path to an SDK, or the canonical name of an SDK.

     -showsdks
           Lists all available SDKs that Xcode knows about, including their canonical names suitable for use
           with -sdk.  Does not initiate a build.

     -list
           Lists the targets and configurations in a project, or the schemes in a workspace. Does not initi-ate initiate
           ate a build.

     -derivedDataPath path
           Overrides the folder that should be used for derived data when performing a build action on a
           scheme in a workspace.

     -resultBundlePath path
           Writes a bundle to the specified path with results from performing a build action on a scheme in
           a workspace.

     -exportArchive
           Specifies that an archive should be exported. Requires -exportFormat, -archivePath, and
           -exportPath.  Cannot be passed along with a build action.

     -exportFormat format
           Specifies the format to which the archive should be exported. Valid formats are IPA (iOS archives
           only), PKG (Mac archives only), and APP.  If not specified, xcodebuild will attempt to auto-detect autodetect
           detect the format as either IPA or PKG.

     -archivePath xcarchivepath
           Specifies the path for the archive produced by the archive action, or specifies the archive that
           should be exported when -exportArchive is passed.

     -exportPath destinationpath
           Specifies the destination for the exported product, including the name of the exported file.

     -exportProvisioningProfile profilename
           Specifies the provisioning profile that should be used when exporting the archive.

     -exportSigningIdentity identityname
           Specifies the application signing identity that should be used when exporting the archive.  If
           possible, this may be inferred from -exportProvisioningProfile.

     -exportInstallerIdentity identityname
           Specifies the installer signing identity that should be used when exporting the archive. If pos-sible, possible,dry
           sible, this may be inferred from -exportSigningIdentity or -exportProvisioningProfile.

     -exportWithOriginalSigningIdentity
           Specifies that the signing identity used to create the archive should be used when exporting the
           archive.

     buildaction ...
           Specify a build action (or actions) to perform on the target. Available build actions are:

           build       Build the target in the build root (SYMROOT).  This is the default build action.

           analyze     Build and analyze a target or scheme from the build root (SYMROOT).  This requires
                       specifying a scheme.

           archive     Archive a scheme from the build root (SYMROOT).  This requires specifying a scheme.

           test        Test a scheme from the build root (SYMROOT).  This requires specifying a scheme and
                       optionally a destination.

           installsrc  Copy the source of the project to the source root (SRCROOT).

           install     Build the target and install it into the target's installation directory in the dis-tribution distribution
                       tribution root (DSTROOT).

           clean       Remove build products and intermediate files from the build root (SYMROOT).

     -xcconfig filename
           Load the build settings defined in filename when building all targets.  These settings will over-ride override
           ride all other settings, including settings passed individually on the command line.

     -dry-run, -n
           Print the commands that would be executed, but do not execute them.

     -skipUnavailableActions
           Skip build actions that cannot be performed instead of failing. This option is only honored if
           -scheme is passed.

     setting=value
           Set the build setting setting to value.

     -userdefault=value
           Set the user default default to value.

     -version
           Display version information for this install of Xcode. Does not initiate a build. When used in
           conjunction with -sdk, the version of the specified SDK is displayed, or all SDKs if -sdk is
           given no argument.  Additionally, a single line of the reported version information may be
           returned if infoitem is specified.

     -usage
           Displays usage information for xcodebuild.
-----------------------------------
   Destinations
     The -destination option takes as its argument a destination specifier describing the device (or
     devices) to use as a destination.  A destination specifier is a single argument consisting of a set of
     comma-separated key=value pairs.  The -destination option may be specified multiple times to cause
     xcodebuild to perform the specified action on multiple destinations.

     Destination specifiers may include the platform key to specify one of the supported destination plat-forms. platforms.
     forms.  There are additional keys which should be supplied depending on the platform of the device you
     are selecting.

     Some devices may take time to look up. The -destination-timeout option can be used to specify the
     amount of time to wait before a device is considered unavailable.  If unspecified, the default timeout
     is 30 seconds.

     Currently, xcodebuild supports these platforms:

     OS X           The local Mac, referred to in the Xcode interface as My Mac, and which supports the fol-lowing following
                    lowing keys:

                    arch  The architecture to use, either x86_64 (the default) or i386.

     iOS            An iOS device, which supports the following keys:

                    name  The name of the device to use.

                    id    The identifier of the device to use, as shown in the Devices tab of the Xcode
                          Organizer.

     iOS Simulator  The iOS Simulator, which supports the following keys:

                    name  The full name of device to simulate, as presented in Xcode's UI.

                    OS    The version of iOS to simulate, such as 6._, or the string latest (the default) to
                          indicate the most recent version of iOS supported by this version of Xcode.

     Some actions (such as building) may be performed without an actual device present.  To build against a
     platform generically instead of a specific device, the destination specifier may be prefixed with the
     optional string "generic/", indicating that the platform should be targeted generically.  An example of
     a generic destination is the "iOS Device" destination displayed in Xcode's UI when no physical iOS
     device is present.
---------------------
 Exporting Archives
   The -exportArchive option specifies that xcodebuild should export the archive specified by -archivePath
   to the format specified by -exportFormat.  The exported product will be placed at the path specified by
   -exportPath.  Re-signing when exporting an archive is optional. The provisioning profile that should be
   embedded in the exported product can be specified by -exportProvisioningProfile.  In some cases, the
   application signing identity that should be used during the export can be determined from the provision-ing provisioning
   ing profile. For cases where that is not possible (including when no provisioning profile is embedded in
   the exported product), an application signing identity can be specified with -exportSigningIdentity.
   When exporting a Mac archive as PKG, an installer signing identity can be used to sign the exported pack-age. package.
   age. This may be inferred from the application signing identity (for instance, if "Developer ID Applica-tion" Application"
   tion" is specified for the application signing identity, "Developer ID Installer" will automatically be
   inferred), but it can be explicitly specified using -exportInstallerIdentity.
--------------------
 Environment Variables
   The following environment variables affect the execution of xcodebuild:

   XCODE_XCCONFIG_FILE
                   Set to a path to a file, build settings in that file will be loaded and used when build-ing building
                   ing all targets.  These settings will override all other settings, including settings
                   passed individually on the command line, and those in the file passed with the -xcconfig
                   option.
-----------------
 Exit Codes
   xcodebuild exits with codes defined by sysexits(3).  It will exit with EX_OK on success.  On failure, it
   will commonly exit with EX_USAGE if any options appear malformed, EX_NOINPUT if any input files cannot be
   found, EX_IOERR if any files cannot be read or written, and EX_SOFTWARE if the commands given to xcode-build xcodebuild
   build fail.  It may exit with other codes in less common scenarios.
---------------------------------
示例
     xcodebuild clean install

              Cleans the build directory; then builds and installs the first target in the Xcode project in
              the directory from which xcodebuild was started.

     xcodebuild -target MyTarget OBJROOT=/Build/MyProj/Obj.root SYMROOT=/Build/MyProj/Sym.root

              Builds the target MyTarget in the Xcode project in the directory from which xcodebuild was
              started, putting intermediate files in the directory /Build/MyProj/Obj.root and the products
              of the build in the directory /Build/MyProj/Sym.root.

     xcodebuild -sdk macosx10.6

              Builds the Xcode project in the directory from which xcodebuild was started against the Mac OS
              X 10.6 SDK.  The canonical names of all available SDKs can be viewed using the -showsdks
              option.

     xcodebuild -workspace MyWorkspace.xcworkspace -scheme MyScheme

              Builds the scheme MyScheme in the Xcode workspace MyWorkspace.xcworkspace.

     xcodebuild -workspace MyWorkspace.xcworkspace -scheme MyScheme archive

              Archives the scheme MyScheme in the Xcode workspace MyWorkspace.xcworkspace.

     xcodebuild -workspace MyWorkspace.xcworkspace -scheme MyScheme -destination 'platform=OS X,arch=x86_64'
              test

              Tests the scheme MyScheme in the Xcode workspace MyWorkspace.xcworkspace using the destination
              described as My Mac 64-bit in Xcode.

     xcodebuild -workspace MyWorkspace.xcworkspace -scheme MyScheme -destination 'platform=iOS
              Simulator,name=iPhone' -destination 'platform=iOS,name=My iPad' test

              Tests the scheme MyScheme in the Xcode workspace MyWorkspace.xcworkspace using both the iOS
              Simulator configured as an iPhone and the the iOS device named My iPad.  (Note that the shell
              requires arguments to be quoted or otherwise escaped if they contain spaces.)

     xcodebuild -workspace MyWorkspace.xcworkspace -scheme MyScheme -destination generic/platform=iOS build

              Builds the scheme MyScheme in the Xcode workspace MyWorkspace.xcworkspace using the generic
              iOS Device destination.

     xcodebuild -exportArchive -exportFormat IPA -archivePath MyMobileApp.xcarchive -exportPath
              MyMobileApp.ipa -exportProvisioningProfile 'MyMobileApp Distribution Profile'

              Exports the archive MyMobileApp.xcarchive as an IPA file to the path MyMobileApp.ipa using the
              provisioning profile MyMobileApp Distribution Profile.

     xcodebuild -exportArchive -exportFormat APP -archivePath MyMacApp.xcarchive -exportPath MyMacApp.pkg
              -exportSigningIdentity 'Developer ID Application: My Team'

              Exports the archive MyMacApp.xcarchive as a PKG file to the path MyMacApp.pkg using the appli-cation application
              cation signing identity Developer ID Application: My Team.  The installer signing identity
              Developer ID Installer: My Team is implicitly used to sign the exported package.

SEE ALSO
     sysexits(3)

Mac OS X                       January 22, 2013                       Mac OS X
```

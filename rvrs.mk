##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=rvrs
ConfigurationName      :=Debug
WorkspacePath          :=/home/boolpae/Dev/rvrs
ProjectPath            :=/home/boolpae/Dev/rvrs
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=SEUNG-HOON LEE
Date                   :=27/02/18
CodeLitePath           :=/home/boolpae/.codelite
LinkerName             :=/usr/bin/g++
SharedObjectLinkerName :=/usr/bin/g++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.i
DebugSwitch            :=-g 
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputFile             :=$(IntermediateDirectory)/$(ProjectName)
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="rvrs.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            :=  
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). $(IncludeSwitch)include 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)pthread $(LibrarySwitch)gearman $(LibrarySwitch)log4cpp $(LibrarySwitch)boost_program_options $(LibrarySwitch)boost_filesystem $(LibrarySwitch)boost_system 
ArLibs                 :=  "pthread" "gearman" "log4cpp" "boost_program_options" "boost_filesystem" "boost_system" 
LibPath                := $(LibraryPathSwitch). 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := /usr/bin/ar rcu
CXX      := /usr/bin/g++
CC       := /usr/bin/gcc
CXXFLAGS :=  -g -O0 -std=c++11 -Wall $(Preprocessors)
CFLAGS   :=  -g -O0 -Wall $(Preprocessors)
ASFLAGS  := 
AS       := /usr/bin/as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/src_common_configuration.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_rvrs_rvrs.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_rvrs_VRCManager.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_rvrs_CallReceiver.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_rvrs_VRClient.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_rvrs_CallExecutor.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_rvrs_CallSignal.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_rvrs_VDClient.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_rvrs_WorkTracer.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_rvrs_STTDeliver.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/src_rvrs_VDCManager.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_rvrs_HAManager.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_rvrs_DBManager.cpp$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

MakeIntermediateDirs:
	@test -d ./Debug || $(MakeDirCommand) ./Debug


$(IntermediateDirectory)/.d:
	@test -d ./Debug || $(MakeDirCommand) ./Debug

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/src_common_configuration.cpp$(ObjectSuffix): src/common/configuration.cpp $(IntermediateDirectory)/src_common_configuration.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/common/configuration.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_common_configuration.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_common_configuration.cpp$(DependSuffix): src/common/configuration.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_common_configuration.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_common_configuration.cpp$(DependSuffix) -MM src/common/configuration.cpp

$(IntermediateDirectory)/src_common_configuration.cpp$(PreprocessSuffix): src/common/configuration.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_common_configuration.cpp$(PreprocessSuffix) src/common/configuration.cpp

$(IntermediateDirectory)/src_rvrs_rvrs.cpp$(ObjectSuffix): src/rvrs/rvrs.cpp $(IntermediateDirectory)/src_rvrs_rvrs.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/rvrs/rvrs.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_rvrs_rvrs.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_rvrs_rvrs.cpp$(DependSuffix): src/rvrs/rvrs.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_rvrs_rvrs.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_rvrs_rvrs.cpp$(DependSuffix) -MM src/rvrs/rvrs.cpp

$(IntermediateDirectory)/src_rvrs_rvrs.cpp$(PreprocessSuffix): src/rvrs/rvrs.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_rvrs_rvrs.cpp$(PreprocessSuffix) src/rvrs/rvrs.cpp

$(IntermediateDirectory)/src_rvrs_VRCManager.cpp$(ObjectSuffix): src/rvrs/VRCManager.cpp $(IntermediateDirectory)/src_rvrs_VRCManager.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/rvrs/VRCManager.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_rvrs_VRCManager.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_rvrs_VRCManager.cpp$(DependSuffix): src/rvrs/VRCManager.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_rvrs_VRCManager.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_rvrs_VRCManager.cpp$(DependSuffix) -MM src/rvrs/VRCManager.cpp

$(IntermediateDirectory)/src_rvrs_VRCManager.cpp$(PreprocessSuffix): src/rvrs/VRCManager.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_rvrs_VRCManager.cpp$(PreprocessSuffix) src/rvrs/VRCManager.cpp

$(IntermediateDirectory)/src_rvrs_CallReceiver.cpp$(ObjectSuffix): src/rvrs/CallReceiver.cpp $(IntermediateDirectory)/src_rvrs_CallReceiver.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/rvrs/CallReceiver.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_rvrs_CallReceiver.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_rvrs_CallReceiver.cpp$(DependSuffix): src/rvrs/CallReceiver.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_rvrs_CallReceiver.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_rvrs_CallReceiver.cpp$(DependSuffix) -MM src/rvrs/CallReceiver.cpp

$(IntermediateDirectory)/src_rvrs_CallReceiver.cpp$(PreprocessSuffix): src/rvrs/CallReceiver.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_rvrs_CallReceiver.cpp$(PreprocessSuffix) src/rvrs/CallReceiver.cpp

$(IntermediateDirectory)/src_rvrs_VRClient.cpp$(ObjectSuffix): src/rvrs/VRClient.cpp $(IntermediateDirectory)/src_rvrs_VRClient.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/rvrs/VRClient.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_rvrs_VRClient.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_rvrs_VRClient.cpp$(DependSuffix): src/rvrs/VRClient.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_rvrs_VRClient.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_rvrs_VRClient.cpp$(DependSuffix) -MM src/rvrs/VRClient.cpp

$(IntermediateDirectory)/src_rvrs_VRClient.cpp$(PreprocessSuffix): src/rvrs/VRClient.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_rvrs_VRClient.cpp$(PreprocessSuffix) src/rvrs/VRClient.cpp

$(IntermediateDirectory)/src_rvrs_CallExecutor.cpp$(ObjectSuffix): src/rvrs/CallExecutor.cpp $(IntermediateDirectory)/src_rvrs_CallExecutor.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/rvrs/CallExecutor.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_rvrs_CallExecutor.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_rvrs_CallExecutor.cpp$(DependSuffix): src/rvrs/CallExecutor.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_rvrs_CallExecutor.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_rvrs_CallExecutor.cpp$(DependSuffix) -MM src/rvrs/CallExecutor.cpp

$(IntermediateDirectory)/src_rvrs_CallExecutor.cpp$(PreprocessSuffix): src/rvrs/CallExecutor.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_rvrs_CallExecutor.cpp$(PreprocessSuffix) src/rvrs/CallExecutor.cpp

$(IntermediateDirectory)/src_rvrs_CallSignal.cpp$(ObjectSuffix): src/rvrs/CallSignal.cpp $(IntermediateDirectory)/src_rvrs_CallSignal.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/rvrs/CallSignal.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_rvrs_CallSignal.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_rvrs_CallSignal.cpp$(DependSuffix): src/rvrs/CallSignal.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_rvrs_CallSignal.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_rvrs_CallSignal.cpp$(DependSuffix) -MM src/rvrs/CallSignal.cpp

$(IntermediateDirectory)/src_rvrs_CallSignal.cpp$(PreprocessSuffix): src/rvrs/CallSignal.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_rvrs_CallSignal.cpp$(PreprocessSuffix) src/rvrs/CallSignal.cpp

$(IntermediateDirectory)/src_rvrs_VDClient.cpp$(ObjectSuffix): src/rvrs/VDClient.cpp $(IntermediateDirectory)/src_rvrs_VDClient.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/rvrs/VDClient.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_rvrs_VDClient.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_rvrs_VDClient.cpp$(DependSuffix): src/rvrs/VDClient.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_rvrs_VDClient.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_rvrs_VDClient.cpp$(DependSuffix) -MM src/rvrs/VDClient.cpp

$(IntermediateDirectory)/src_rvrs_VDClient.cpp$(PreprocessSuffix): src/rvrs/VDClient.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_rvrs_VDClient.cpp$(PreprocessSuffix) src/rvrs/VDClient.cpp

$(IntermediateDirectory)/src_rvrs_WorkTracer.cpp$(ObjectSuffix): src/rvrs/WorkTracer.cpp $(IntermediateDirectory)/src_rvrs_WorkTracer.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/rvrs/WorkTracer.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_rvrs_WorkTracer.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_rvrs_WorkTracer.cpp$(DependSuffix): src/rvrs/WorkTracer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_rvrs_WorkTracer.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_rvrs_WorkTracer.cpp$(DependSuffix) -MM src/rvrs/WorkTracer.cpp

$(IntermediateDirectory)/src_rvrs_WorkTracer.cpp$(PreprocessSuffix): src/rvrs/WorkTracer.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_rvrs_WorkTracer.cpp$(PreprocessSuffix) src/rvrs/WorkTracer.cpp

$(IntermediateDirectory)/src_rvrs_STTDeliver.cpp$(ObjectSuffix): src/rvrs/STTDeliver.cpp $(IntermediateDirectory)/src_rvrs_STTDeliver.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/rvrs/STTDeliver.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_rvrs_STTDeliver.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_rvrs_STTDeliver.cpp$(DependSuffix): src/rvrs/STTDeliver.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_rvrs_STTDeliver.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_rvrs_STTDeliver.cpp$(DependSuffix) -MM src/rvrs/STTDeliver.cpp

$(IntermediateDirectory)/src_rvrs_STTDeliver.cpp$(PreprocessSuffix): src/rvrs/STTDeliver.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_rvrs_STTDeliver.cpp$(PreprocessSuffix) src/rvrs/STTDeliver.cpp

$(IntermediateDirectory)/src_rvrs_VDCManager.cpp$(ObjectSuffix): src/rvrs/VDCManager.cpp $(IntermediateDirectory)/src_rvrs_VDCManager.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/rvrs/VDCManager.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_rvrs_VDCManager.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_rvrs_VDCManager.cpp$(DependSuffix): src/rvrs/VDCManager.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_rvrs_VDCManager.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_rvrs_VDCManager.cpp$(DependSuffix) -MM src/rvrs/VDCManager.cpp

$(IntermediateDirectory)/src_rvrs_VDCManager.cpp$(PreprocessSuffix): src/rvrs/VDCManager.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_rvrs_VDCManager.cpp$(PreprocessSuffix) src/rvrs/VDCManager.cpp

$(IntermediateDirectory)/src_rvrs_HAManager.cpp$(ObjectSuffix): src/rvrs/HAManager.cpp $(IntermediateDirectory)/src_rvrs_HAManager.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/rvrs/HAManager.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_rvrs_HAManager.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_rvrs_HAManager.cpp$(DependSuffix): src/rvrs/HAManager.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_rvrs_HAManager.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_rvrs_HAManager.cpp$(DependSuffix) -MM src/rvrs/HAManager.cpp

$(IntermediateDirectory)/src_rvrs_HAManager.cpp$(PreprocessSuffix): src/rvrs/HAManager.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_rvrs_HAManager.cpp$(PreprocessSuffix) src/rvrs/HAManager.cpp

$(IntermediateDirectory)/src_rvrs_DBManager.cpp$(ObjectSuffix): src/rvrs/DBManager.cpp $(IntermediateDirectory)/src_rvrs_DBManager.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/rvrs/DBManager.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_rvrs_DBManager.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_rvrs_DBManager.cpp$(DependSuffix): src/rvrs/DBManager.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_rvrs_DBManager.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_rvrs_DBManager.cpp$(DependSuffix) -MM src/rvrs/DBManager.cpp

$(IntermediateDirectory)/src_rvrs_DBManager.cpp$(PreprocessSuffix): src/rvrs/DBManager.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_rvrs_DBManager.cpp$(PreprocessSuffix) src/rvrs/DBManager.cpp


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./Debug/



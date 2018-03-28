##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=rvrs
ConfigurationName      :=Debug
WorkspacePath          := "/home/boolpae/Dev/rvrs"
ProjectPath            := "/home/boolpae/Dev/rvrs"
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=SEUNG-HOON LEE
Date                   :=27/03/18
CodeLitePath           :="/home/boolpae/.codelite"
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
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). $(IncludeSwitch)include $(IncludeSwitch)/usr/include/zdb 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)pthread $(LibrarySwitch)gearman $(LibrarySwitch)log4cpp $(LibrarySwitch)boost_program_options $(LibrarySwitch)boost_filesystem $(LibrarySwitch)boost_system $(LibrarySwitch)zdb 
ArLibs                 :=  "pthread" "gearman" "log4cpp" "boost_program_options" "boost_filesystem" "boost_system" "zdb" 
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
Objects0=$(IntermediateDirectory)/common_configuration.cpp$(ObjectSuffix) $(IntermediateDirectory)/rvrs_rvrs.cpp$(ObjectSuffix) $(IntermediateDirectory)/rvrs_VRCManager.cpp$(ObjectSuffix) $(IntermediateDirectory)/rvrs_CallReceiver.cpp$(ObjectSuffix) $(IntermediateDirectory)/rvrs_VRClient.cpp$(ObjectSuffix) $(IntermediateDirectory)/rvrs_CallExecutor.cpp$(ObjectSuffix) $(IntermediateDirectory)/rvrs_CallSignal.cpp$(ObjectSuffix) $(IntermediateDirectory)/rvrs_VDClient.cpp$(ObjectSuffix) $(IntermediateDirectory)/rvrs_WorkTracer.cpp$(ObjectSuffix) $(IntermediateDirectory)/rvrs_STTDeliver.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/rvrs_VDCManager.cpp$(ObjectSuffix) $(IntermediateDirectory)/rvrs_HAManager.cpp$(ObjectSuffix) $(IntermediateDirectory)/rvrs_RT2DB.cpp$(ObjectSuffix) 



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
$(IntermediateDirectory)/common_configuration.cpp$(ObjectSuffix): src/common/configuration.cpp $(IntermediateDirectory)/common_configuration.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/common/configuration.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/common_configuration.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/common_configuration.cpp$(DependSuffix): src/common/configuration.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/common_configuration.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/common_configuration.cpp$(DependSuffix) -MM "src/common/configuration.cpp"

$(IntermediateDirectory)/common_configuration.cpp$(PreprocessSuffix): src/common/configuration.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/common_configuration.cpp$(PreprocessSuffix) "src/common/configuration.cpp"

$(IntermediateDirectory)/rvrs_rvrs.cpp$(ObjectSuffix): src/rvrs/rvrs.cpp $(IntermediateDirectory)/rvrs_rvrs.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/rvrs/rvrs.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/rvrs_rvrs.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/rvrs_rvrs.cpp$(DependSuffix): src/rvrs/rvrs.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/rvrs_rvrs.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/rvrs_rvrs.cpp$(DependSuffix) -MM "src/rvrs/rvrs.cpp"

$(IntermediateDirectory)/rvrs_rvrs.cpp$(PreprocessSuffix): src/rvrs/rvrs.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/rvrs_rvrs.cpp$(PreprocessSuffix) "src/rvrs/rvrs.cpp"

$(IntermediateDirectory)/rvrs_VRCManager.cpp$(ObjectSuffix): src/rvrs/VRCManager.cpp $(IntermediateDirectory)/rvrs_VRCManager.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/rvrs/VRCManager.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/rvrs_VRCManager.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/rvrs_VRCManager.cpp$(DependSuffix): src/rvrs/VRCManager.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/rvrs_VRCManager.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/rvrs_VRCManager.cpp$(DependSuffix) -MM "src/rvrs/VRCManager.cpp"

$(IntermediateDirectory)/rvrs_VRCManager.cpp$(PreprocessSuffix): src/rvrs/VRCManager.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/rvrs_VRCManager.cpp$(PreprocessSuffix) "src/rvrs/VRCManager.cpp"

$(IntermediateDirectory)/rvrs_CallReceiver.cpp$(ObjectSuffix): src/rvrs/CallReceiver.cpp $(IntermediateDirectory)/rvrs_CallReceiver.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/rvrs/CallReceiver.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/rvrs_CallReceiver.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/rvrs_CallReceiver.cpp$(DependSuffix): src/rvrs/CallReceiver.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/rvrs_CallReceiver.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/rvrs_CallReceiver.cpp$(DependSuffix) -MM "src/rvrs/CallReceiver.cpp"

$(IntermediateDirectory)/rvrs_CallReceiver.cpp$(PreprocessSuffix): src/rvrs/CallReceiver.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/rvrs_CallReceiver.cpp$(PreprocessSuffix) "src/rvrs/CallReceiver.cpp"

$(IntermediateDirectory)/rvrs_VRClient.cpp$(ObjectSuffix): src/rvrs/VRClient.cpp $(IntermediateDirectory)/rvrs_VRClient.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/rvrs/VRClient.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/rvrs_VRClient.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/rvrs_VRClient.cpp$(DependSuffix): src/rvrs/VRClient.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/rvrs_VRClient.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/rvrs_VRClient.cpp$(DependSuffix) -MM "src/rvrs/VRClient.cpp"

$(IntermediateDirectory)/rvrs_VRClient.cpp$(PreprocessSuffix): src/rvrs/VRClient.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/rvrs_VRClient.cpp$(PreprocessSuffix) "src/rvrs/VRClient.cpp"

$(IntermediateDirectory)/rvrs_CallExecutor.cpp$(ObjectSuffix): src/rvrs/CallExecutor.cpp $(IntermediateDirectory)/rvrs_CallExecutor.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/rvrs/CallExecutor.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/rvrs_CallExecutor.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/rvrs_CallExecutor.cpp$(DependSuffix): src/rvrs/CallExecutor.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/rvrs_CallExecutor.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/rvrs_CallExecutor.cpp$(DependSuffix) -MM "src/rvrs/CallExecutor.cpp"

$(IntermediateDirectory)/rvrs_CallExecutor.cpp$(PreprocessSuffix): src/rvrs/CallExecutor.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/rvrs_CallExecutor.cpp$(PreprocessSuffix) "src/rvrs/CallExecutor.cpp"

$(IntermediateDirectory)/rvrs_CallSignal.cpp$(ObjectSuffix): src/rvrs/CallSignal.cpp $(IntermediateDirectory)/rvrs_CallSignal.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/rvrs/CallSignal.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/rvrs_CallSignal.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/rvrs_CallSignal.cpp$(DependSuffix): src/rvrs/CallSignal.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/rvrs_CallSignal.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/rvrs_CallSignal.cpp$(DependSuffix) -MM "src/rvrs/CallSignal.cpp"

$(IntermediateDirectory)/rvrs_CallSignal.cpp$(PreprocessSuffix): src/rvrs/CallSignal.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/rvrs_CallSignal.cpp$(PreprocessSuffix) "src/rvrs/CallSignal.cpp"

$(IntermediateDirectory)/rvrs_VDClient.cpp$(ObjectSuffix): src/rvrs/VDClient.cpp $(IntermediateDirectory)/rvrs_VDClient.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/rvrs/VDClient.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/rvrs_VDClient.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/rvrs_VDClient.cpp$(DependSuffix): src/rvrs/VDClient.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/rvrs_VDClient.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/rvrs_VDClient.cpp$(DependSuffix) -MM "src/rvrs/VDClient.cpp"

$(IntermediateDirectory)/rvrs_VDClient.cpp$(PreprocessSuffix): src/rvrs/VDClient.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/rvrs_VDClient.cpp$(PreprocessSuffix) "src/rvrs/VDClient.cpp"

$(IntermediateDirectory)/rvrs_WorkTracer.cpp$(ObjectSuffix): src/rvrs/WorkTracer.cpp $(IntermediateDirectory)/rvrs_WorkTracer.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/rvrs/WorkTracer.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/rvrs_WorkTracer.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/rvrs_WorkTracer.cpp$(DependSuffix): src/rvrs/WorkTracer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/rvrs_WorkTracer.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/rvrs_WorkTracer.cpp$(DependSuffix) -MM "src/rvrs/WorkTracer.cpp"

$(IntermediateDirectory)/rvrs_WorkTracer.cpp$(PreprocessSuffix): src/rvrs/WorkTracer.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/rvrs_WorkTracer.cpp$(PreprocessSuffix) "src/rvrs/WorkTracer.cpp"

$(IntermediateDirectory)/rvrs_STTDeliver.cpp$(ObjectSuffix): src/rvrs/STTDeliver.cpp $(IntermediateDirectory)/rvrs_STTDeliver.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/rvrs/STTDeliver.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/rvrs_STTDeliver.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/rvrs_STTDeliver.cpp$(DependSuffix): src/rvrs/STTDeliver.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/rvrs_STTDeliver.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/rvrs_STTDeliver.cpp$(DependSuffix) -MM "src/rvrs/STTDeliver.cpp"

$(IntermediateDirectory)/rvrs_STTDeliver.cpp$(PreprocessSuffix): src/rvrs/STTDeliver.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/rvrs_STTDeliver.cpp$(PreprocessSuffix) "src/rvrs/STTDeliver.cpp"

$(IntermediateDirectory)/rvrs_VDCManager.cpp$(ObjectSuffix): src/rvrs/VDCManager.cpp $(IntermediateDirectory)/rvrs_VDCManager.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/rvrs/VDCManager.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/rvrs_VDCManager.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/rvrs_VDCManager.cpp$(DependSuffix): src/rvrs/VDCManager.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/rvrs_VDCManager.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/rvrs_VDCManager.cpp$(DependSuffix) -MM "src/rvrs/VDCManager.cpp"

$(IntermediateDirectory)/rvrs_VDCManager.cpp$(PreprocessSuffix): src/rvrs/VDCManager.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/rvrs_VDCManager.cpp$(PreprocessSuffix) "src/rvrs/VDCManager.cpp"

$(IntermediateDirectory)/rvrs_HAManager.cpp$(ObjectSuffix): src/rvrs/HAManager.cpp $(IntermediateDirectory)/rvrs_HAManager.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/rvrs/HAManager.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/rvrs_HAManager.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/rvrs_HAManager.cpp$(DependSuffix): src/rvrs/HAManager.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/rvrs_HAManager.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/rvrs_HAManager.cpp$(DependSuffix) -MM "src/rvrs/HAManager.cpp"

$(IntermediateDirectory)/rvrs_HAManager.cpp$(PreprocessSuffix): src/rvrs/HAManager.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/rvrs_HAManager.cpp$(PreprocessSuffix) "src/rvrs/HAManager.cpp"

$(IntermediateDirectory)/rvrs_RT2DB.cpp$(ObjectSuffix): src/rvrs/RT2DB.cpp $(IntermediateDirectory)/rvrs_RT2DB.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/rvrs/RT2DB.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/rvrs_RT2DB.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/rvrs_RT2DB.cpp$(DependSuffix): src/rvrs/RT2DB.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/rvrs_RT2DB.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/rvrs_RT2DB.cpp$(DependSuffix) -MM "src/rvrs/RT2DB.cpp"

$(IntermediateDirectory)/rvrs_RT2DB.cpp$(PreprocessSuffix): src/rvrs/RT2DB.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/rvrs_RT2DB.cpp$(PreprocessSuffix) "src/rvrs/RT2DB.cpp"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./Debug/



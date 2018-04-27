##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=stas
ConfigurationName      :=Debug
WorkspacePath          := "/home/boolpae/Dev/rvrs"
ProjectPath            := "/home/boolpae/Dev/rvrs"
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=SEUNG-HOON LEE
Date                   :=27/04/18
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
ObjectsFileList        :="stas.txt"
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
Objects0=$(IntermediateDirectory)/common_configuration.cpp$(ObjectSuffix) $(IntermediateDirectory)/stas_CallExecutor.cpp$(ObjectSuffix) $(IntermediateDirectory)/stas_CallReceiver.cpp$(ObjectSuffix) $(IntermediateDirectory)/stas_CallSignal.cpp$(ObjectSuffix) $(IntermediateDirectory)/stas_HAManager.cpp$(ObjectSuffix) $(IntermediateDirectory)/stas_RT2DB.cpp$(ObjectSuffix) $(IntermediateDirectory)/stas_stas.cpp$(ObjectSuffix) $(IntermediateDirectory)/stas_STTDeliver.cpp$(ObjectSuffix) $(IntermediateDirectory)/stas_VDClient.cpp$(ObjectSuffix) $(IntermediateDirectory)/stas_VDCManager.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/stas_VRClient.cpp$(ObjectSuffix) $(IntermediateDirectory)/stas_VRCManager.cpp$(ObjectSuffix) $(IntermediateDirectory)/stas_WorkTracer.cpp$(ObjectSuffix) 



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

$(IntermediateDirectory)/stas_CallExecutor.cpp$(ObjectSuffix): src/stas/CallExecutor.cpp $(IntermediateDirectory)/stas_CallExecutor.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/stas/CallExecutor.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/stas_CallExecutor.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/stas_CallExecutor.cpp$(DependSuffix): src/stas/CallExecutor.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/stas_CallExecutor.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/stas_CallExecutor.cpp$(DependSuffix) -MM "src/stas/CallExecutor.cpp"

$(IntermediateDirectory)/stas_CallExecutor.cpp$(PreprocessSuffix): src/stas/CallExecutor.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/stas_CallExecutor.cpp$(PreprocessSuffix) "src/stas/CallExecutor.cpp"

$(IntermediateDirectory)/stas_CallReceiver.cpp$(ObjectSuffix): src/stas/CallReceiver.cpp $(IntermediateDirectory)/stas_CallReceiver.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/stas/CallReceiver.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/stas_CallReceiver.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/stas_CallReceiver.cpp$(DependSuffix): src/stas/CallReceiver.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/stas_CallReceiver.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/stas_CallReceiver.cpp$(DependSuffix) -MM "src/stas/CallReceiver.cpp"

$(IntermediateDirectory)/stas_CallReceiver.cpp$(PreprocessSuffix): src/stas/CallReceiver.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/stas_CallReceiver.cpp$(PreprocessSuffix) "src/stas/CallReceiver.cpp"

$(IntermediateDirectory)/stas_CallSignal.cpp$(ObjectSuffix): src/stas/CallSignal.cpp $(IntermediateDirectory)/stas_CallSignal.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/stas/CallSignal.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/stas_CallSignal.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/stas_CallSignal.cpp$(DependSuffix): src/stas/CallSignal.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/stas_CallSignal.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/stas_CallSignal.cpp$(DependSuffix) -MM "src/stas/CallSignal.cpp"

$(IntermediateDirectory)/stas_CallSignal.cpp$(PreprocessSuffix): src/stas/CallSignal.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/stas_CallSignal.cpp$(PreprocessSuffix) "src/stas/CallSignal.cpp"

$(IntermediateDirectory)/stas_HAManager.cpp$(ObjectSuffix): src/stas/HAManager.cpp $(IntermediateDirectory)/stas_HAManager.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/stas/HAManager.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/stas_HAManager.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/stas_HAManager.cpp$(DependSuffix): src/stas/HAManager.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/stas_HAManager.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/stas_HAManager.cpp$(DependSuffix) -MM "src/stas/HAManager.cpp"

$(IntermediateDirectory)/stas_HAManager.cpp$(PreprocessSuffix): src/stas/HAManager.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/stas_HAManager.cpp$(PreprocessSuffix) "src/stas/HAManager.cpp"

$(IntermediateDirectory)/stas_RT2DB.cpp$(ObjectSuffix): src/stas/RT2DB.cpp $(IntermediateDirectory)/stas_RT2DB.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/stas/RT2DB.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/stas_RT2DB.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/stas_RT2DB.cpp$(DependSuffix): src/stas/RT2DB.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/stas_RT2DB.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/stas_RT2DB.cpp$(DependSuffix) -MM "src/stas/RT2DB.cpp"

$(IntermediateDirectory)/stas_RT2DB.cpp$(PreprocessSuffix): src/stas/RT2DB.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/stas_RT2DB.cpp$(PreprocessSuffix) "src/stas/RT2DB.cpp"

$(IntermediateDirectory)/stas_stas.cpp$(ObjectSuffix): src/stas/stas.cpp $(IntermediateDirectory)/stas_stas.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/stas/stas.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/stas_stas.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/stas_stas.cpp$(DependSuffix): src/stas/stas.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/stas_stas.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/stas_stas.cpp$(DependSuffix) -MM "src/stas/stas.cpp"

$(IntermediateDirectory)/stas_stas.cpp$(PreprocessSuffix): src/stas/stas.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/stas_stas.cpp$(PreprocessSuffix) "src/stas/stas.cpp"

$(IntermediateDirectory)/stas_STTDeliver.cpp$(ObjectSuffix): src/stas/STTDeliver.cpp $(IntermediateDirectory)/stas_STTDeliver.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/stas/STTDeliver.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/stas_STTDeliver.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/stas_STTDeliver.cpp$(DependSuffix): src/stas/STTDeliver.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/stas_STTDeliver.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/stas_STTDeliver.cpp$(DependSuffix) -MM "src/stas/STTDeliver.cpp"

$(IntermediateDirectory)/stas_STTDeliver.cpp$(PreprocessSuffix): src/stas/STTDeliver.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/stas_STTDeliver.cpp$(PreprocessSuffix) "src/stas/STTDeliver.cpp"

$(IntermediateDirectory)/stas_VDClient.cpp$(ObjectSuffix): src/stas/VDClient.cpp $(IntermediateDirectory)/stas_VDClient.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/stas/VDClient.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/stas_VDClient.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/stas_VDClient.cpp$(DependSuffix): src/stas/VDClient.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/stas_VDClient.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/stas_VDClient.cpp$(DependSuffix) -MM "src/stas/VDClient.cpp"

$(IntermediateDirectory)/stas_VDClient.cpp$(PreprocessSuffix): src/stas/VDClient.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/stas_VDClient.cpp$(PreprocessSuffix) "src/stas/VDClient.cpp"

$(IntermediateDirectory)/stas_VDCManager.cpp$(ObjectSuffix): src/stas/VDCManager.cpp $(IntermediateDirectory)/stas_VDCManager.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/stas/VDCManager.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/stas_VDCManager.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/stas_VDCManager.cpp$(DependSuffix): src/stas/VDCManager.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/stas_VDCManager.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/stas_VDCManager.cpp$(DependSuffix) -MM "src/stas/VDCManager.cpp"

$(IntermediateDirectory)/stas_VDCManager.cpp$(PreprocessSuffix): src/stas/VDCManager.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/stas_VDCManager.cpp$(PreprocessSuffix) "src/stas/VDCManager.cpp"

$(IntermediateDirectory)/stas_VRClient.cpp$(ObjectSuffix): src/stas/VRClient.cpp $(IntermediateDirectory)/stas_VRClient.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/stas/VRClient.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/stas_VRClient.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/stas_VRClient.cpp$(DependSuffix): src/stas/VRClient.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/stas_VRClient.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/stas_VRClient.cpp$(DependSuffix) -MM "src/stas/VRClient.cpp"

$(IntermediateDirectory)/stas_VRClient.cpp$(PreprocessSuffix): src/stas/VRClient.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/stas_VRClient.cpp$(PreprocessSuffix) "src/stas/VRClient.cpp"

$(IntermediateDirectory)/stas_VRCManager.cpp$(ObjectSuffix): src/stas/VRCManager.cpp $(IntermediateDirectory)/stas_VRCManager.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/stas/VRCManager.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/stas_VRCManager.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/stas_VRCManager.cpp$(DependSuffix): src/stas/VRCManager.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/stas_VRCManager.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/stas_VRCManager.cpp$(DependSuffix) -MM "src/stas/VRCManager.cpp"

$(IntermediateDirectory)/stas_VRCManager.cpp$(PreprocessSuffix): src/stas/VRCManager.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/stas_VRCManager.cpp$(PreprocessSuffix) "src/stas/VRCManager.cpp"

$(IntermediateDirectory)/stas_WorkTracer.cpp$(ObjectSuffix): src/stas/WorkTracer.cpp $(IntermediateDirectory)/stas_WorkTracer.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/boolpae/Dev/rvrs/src/stas/WorkTracer.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/stas_WorkTracer.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/stas_WorkTracer.cpp$(DependSuffix): src/stas/WorkTracer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/stas_WorkTracer.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/stas_WorkTracer.cpp$(DependSuffix) -MM "src/stas/WorkTracer.cpp"

$(IntermediateDirectory)/stas_WorkTracer.cpp$(PreprocessSuffix): src/stas/WorkTracer.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/stas_WorkTracer.cpp$(PreprocessSuffix) "src/stas/WorkTracer.cpp"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./Debug/



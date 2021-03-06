# _api Python module

set(MODULE_TEMPLATE src/api.cpp.in)

# Files containing export definitions, these are automatically processed
# -- Do NOT sort this list. The order defines the order in which the export
# definitions occur and some depend on their base classes being exported first
set(EXPORT_FILES
    src/Exports/IAlgorithm.cpp
    src/Exports/AlgorithmObserver.cpp
    src/Exports/AlgorithmHistory.cpp
    src/Exports/CatalogManager.cpp
    src/Exports/CatalogSession.cpp
    src/Exports/DeprecatedAlgorithmChecker.cpp
    src/Exports/Algorithm.cpp
    src/Exports/DataProcessorAlgorithm.cpp
    src/Exports/AlgorithmFactory.cpp
    src/Exports/AlgorithmFactoryObserver.cpp
    src/Exports/AlgorithmManager.cpp
    src/Exports/AnalysisDataService.cpp
    src/Exports/FileProperty.cpp
    src/Exports/MultipleFileProperty.cpp
    src/Exports/FrameworkManager.cpp
    src/Exports/ISpectrum.cpp
    src/Exports/IEventList.cpp
    src/Exports/WorkspaceHistory.cpp
    src/Exports/ExperimentInfo.cpp
    src/Exports/MultipleExperimentInfos.cpp
    src/Exports/Workspace.cpp
    src/Exports/IWorkspaceProperty.cpp
    src/Exports/WorkspaceProperty.cpp
    src/Exports/ITableWorkspace.cpp
    src/Exports/ITableWorkspaceProperty.cpp
    src/Exports/ISplittersWorkspace.cpp
    src/Exports/MDGeometry.cpp
    src/Exports/IMDWorkspace.cpp
    src/Exports/IMDWorkspaceProperty.cpp
    src/Exports/IMDHistoWorkspace.cpp
    src/Exports/IMDHistoWorkspaceProperty.cpp
    src/Exports/IMDEventWorkspace.cpp
    src/Exports/IMDEventWorkspaceProperty.cpp
    src/Exports/MatrixWorkspace.cpp
    src/Exports/MatrixWorkspaceProperty.cpp
    src/Exports/IEventWorkspace.cpp
    src/Exports/IEventWorkspaceProperty.cpp
    src/Exports/IMaskWorkspace.cpp
    src/Exports/IPeaksWorkspace.cpp
    src/Exports/IPeaksWorkspaceProperty.cpp
    src/Exports/BinaryOperations.cpp
    src/Exports/WorkspaceGroup.cpp
    src/Exports/WorkspaceGroupProperty.cpp
    src/Exports/WorkspaceValidators.cpp
    src/Exports/ADSValidator.cpp
    src/Exports/InstrumentValidator.cpp
    src/Exports/OrientedLatticeValidator.cpp
    src/Exports/Axis.cpp
    src/Exports/IPeak.cpp
    src/Exports/BoxController.cpp
    src/Exports/FileFinder.cpp
    src/Exports/FileLoaderRegistry.cpp
    src/Exports/Sample.cpp
    src/Exports/ScriptRepository.cpp
    src/Exports/ScriptRepositoryFactory.cpp
    src/Exports/SpectrumInfo.cpp
    src/Exports/Run.cpp
    src/Exports/WorkspaceFactory.cpp
    src/Exports/IFunction.cpp
    src/Exports/IFunction1D.cpp
    src/Exports/IPeakFunction.cpp
    src/Exports/CompositeFunction.cpp
    src/Exports/Jacobian.cpp
    src/Exports/FunctionFactory.cpp
    src/Exports/Progress.cpp
    src/Exports/Projection.cpp
    src/Exports/FunctionProperty.cpp
    src/Exports/AlgorithmProperty.cpp
    src/Exports/MultiDomainFunction.cpp
    src/Exports/SpectrumDefinition.cpp
    src/Exports/SpectrumInfoItem.cpp
    src/Exports/SpectrumInfoIterator.cpp
    src/Exports/SpectrumInfoPythonIterator.cpp
    src/Exports/AnalysisDataServiceObserver.cpp
    src/Exports/Citation.cpp)

set(MODULE_DEFINITION ${CMAKE_CURRENT_BINARY_DIR}/api.cpp)
create_module(${MODULE_TEMPLATE} ${MODULE_DEFINITION} ${EXPORT_FILES})

# Helper code
set(SRC_FILES
    src/Algorithms/AlgorithmObserverAdapter.cpp
    src/Algorithms/RunPythonScript.cpp
    src/FitFunctions/IFunctionAdapter.cpp
    src/FitFunctions/IFunction1DAdapter.cpp
    src/FitFunctions/IPeakFunctionAdapter.cpp
    src/PythonAlgorithm/AlgorithmAdapter.cpp
    src/PythonAlgorithm/DataProcessorAdapter.cpp
    src/CloneMatrixWorkspace.cpp
    src/Exports/AlgorithmFactoryObserverAdapter.cpp
    src/Exports/AnalysisDataServiceObserverAdapter.cpp)

set(INC_FILES
    inc/MantidPythonInterface/api/Algorithms/AlgorithmFactoryObserverAdapter.h
    inc/MantidPythonInterface/api/Algorithms/AlgorithmObserverAdapter.h
    inc/MantidPythonInterface/api/Algorithms/RunPythonScript.h
    inc/MantidPythonInterface/api/FitFunctions/IFunctionAdapter.h
    inc/MantidPythonInterface/api/FitFunctions/IFunction1DAdapter.h
    inc/MantidPythonInterface/api/FitFunctions/IPeakFunctionAdapter.h
    inc/MantidPythonInterface/api/PythonAlgorithm/AlgorithmAdapter.h
    inc/MantidPythonInterface/api/PythonAlgorithm/DataProcessorAdapter.h
    inc/MantidPythonInterface/api/AnalysisDataServiceObserverAdapter.h
    inc/MantidPythonInterface/api/BinaryOperations.h
    inc/MantidPythonInterface/api/CloneMatrixWorkspace.h
    inc/MantidPythonInterface/api/WorkspacePropertyExporter.h
    inc/MantidPythonInterface/api/SpectrumInfoPythonIterator.h)

# Create the target for this directory

add_library(PythonAPIModule
            ${EXPORT_FILES}
            ${MODULE_DEFINITION}
            ${SRC_FILES}
            ${INC_FILES}
            ${PYTHON_INSTALL_FILES})
set_python_properties(PythonAPIModule _api)

target_include_directories(PythonAPIModule PUBLIC inc)

# Add the required dependencies
target_link_libraries(PythonAPIModule
                      PRIVATE ${TCMALLOC_LIBRARIES_LINKTIME}
                              PythonInterfaceCore
                              PythonGeometryModule
                              PythonKernelModule
                              API
                              Geometry
                              HistogramData
                              Indexing
                              Kernel
                              Types
                              ${PYTHON_LIBRARIES}
                              ${POCO_LIBRARIES}
                              ${Boost_LIBRARIES})

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set_target_properties(
    PythonAPIModule
    PROPERTIES
      INSTALL_RPATH
      "@loader_path/../../../MacOS;@loader_path/../kernel/;@loader_path/../geometry/"
    )
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  set_target_properties(
    PythonAPIModule
    PROPERTIES
      INSTALL_RPATH
      "\$ORIGIN/../../../${LIB_DIR};\$ORIGIN/../kernel/;\$ORIGIN/../geometry/")
endif()

# Installation settings
mtd_install_targets(TARGETS
                    PythonAPIModule
                    INSTALL_DIRS
                    ${LIB_DIR}/mantid/api
                    ${WORKBENCH_LIB_DIR}/mantid/api)

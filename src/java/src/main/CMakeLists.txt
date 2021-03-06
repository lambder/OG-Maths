#
# Copyright (C) 2013 - present by OpenGamma Inc. and the OpenGamma group of companies
#
# Please see distribution for license.
#

set(JPACKAGEFQN "com.opengamma.maths")
fqn_to_dir(JPACKAGEDIR ${JPACKAGEFQN})

set(JAVA_FILES
    DOGMA.java
    materialisers/Materialisers.java
    helpers/Catchers.java
    helpers/DenseMemoryManipulation.java
    helpers/FuzzyEquals.java
    helpers/Iss.java
    helpers/MatrixPrimitiveUtils.java
    helpers/TestGroups.java
    logging/Logger.java
    logging/ILogger.java
    logging/Slf4jLogger.java
    logging/StderrLogger.java
    nodes/ACOS.java
    nodes/ASINH.java
    nodes/ATAN.java
    nodes/COPY.java
    nodes/COS.java
    nodes/CTRANSPOSE.java
    nodes/EXP.java
    nodes/INV.java
    nodes/LU.java
    nodes/MTIMES.java
    nodes/MLDIVIDE.java
    nodes/NEGATE.java
    nodes/NODE.java
    nodes/NORM2.java
    nodes/PINV.java
    nodes/PLUS.java
    nodes/RDIVIDE.java
    nodes/SELECTRESULT.java
    nodes/SIN.java
    nodes/SINH.java
    nodes/SVD.java
    nodes/TAN.java
    nodes/TANH.java
    nodes/TIMES.java
    nodes/TRANSPOSE.java
    nativeloader/SupportedInstructionSet.java
    nativeloader/NativeLibraries.java
    nativeloader/NativeLibrariesSelfTest.java
    exceptions/MathsException.java
    exceptions/MathsExceptionIllegalArgument.java
    exceptions/MathsExceptionNative.java
    exceptions/MathsExceptionNativeComputation.java
    exceptions/MathsExceptionNativeConversion.java
    exceptions/MathsExceptionNativeUnspecified.java
    exceptions/MathsExceptionNonConformance.java
    exceptions/MathsExceptionNotImplemented.java
    exceptions/MathsExceptionNullPointer.java
    exceptions/MathsExceptionOnInitialization.java
    exceptions/MathsExceptionUnsupportedPlatform.java
    datacontainers/matrix/OGSparseMatrix.java
    datacontainers/matrix/OGComplexDiagonalMatrix.java
    datacontainers/matrix/OGComplexDenseMatrix.java
    datacontainers/matrix/OGRealDiagonalMatrix.java
    datacontainers/matrix/OGDiagonalMatrix.java
    datacontainers/matrix/OGDenseMatrix.java
    datacontainers/matrix/OGComplexSparseMatrix.java
    datacontainers/matrix/OGLogicalMatrix.java
    datacontainers/matrix/OGRealDenseMatrix.java
    datacontainers/matrix/OGRealSparseMatrix.java
    datacontainers/matrix/OGArray.java
    datacontainers/ExprEnum.java
    datacontainers/OGTerminal.java
    datacontainers/other/OGResult.java
    datacontainers/other/ComplexArrayContainer.java
    datacontainers/other/OGSVDResult.java
    datacontainers/other/OGLUResult.java
    datacontainers/lazy/OGExpr.java
    datacontainers/lazy/OGExprVariadicReturn.java
    datacontainers/OGNumeric.java
    datacontainers/scalar/OGScalar.java
    datacontainers/scalar/OGRealScalar.java
    datacontainers/scalar/OGComplexScalar.java
    datacontainers/scalar/OGIntegerScalar.java
    fuzzer/DataCreator.java
    fuzzer/Fuzzer.java
    fuzzer/FuzzerMain.java
    fuzzer/FuzzingThread.java
    fuzzer/ThreadedTreeFuzzer.java
    fuzzer/TreeFuzzer.java
    fuzzer/FixedSizeFIFO.java
    fuzzer/FuzzerResult.java
    fuzzer/TransformAnnotationFuzzOnly.java
    )

if(BITS64)
  set(JAVA_FILES
      ${JAVA_FILES}
      mem/AlignedAlignedWriteBuffer.java
      mem/AVXAlignedMemoryBlock.java
      mem/InvalidAccessException.java
      mem/Stdlib.java
      )
endif()

foreach(JFILE ${JAVA_FILES})
  set(JAR_SOURCES ${JAR_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/java/${JPACKAGEDIR}/${JFILE})
endforeach()


#need these to get the external repo root dir locations
include(${IZY_BUILDINFO})
include(${LAPACK_BUILDINFO})

# This cache is used to trigger dependencies in add_jar
set(jar_licenses ${licensing_dirs} CACHE INTERNAL "The jar licenses")

# This sorts out the licensing directory that gets bundled into the jar root
add_custom_target (licensing_dirs
# root level license dir (license_root_dir)
COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/licenses
# where OG-Maths licenses will live (license_root_dir/licenses)
COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/licenses/licenses
# where OG-Izy licenses will live (license_root_dir/licenses/OG-Izy)
COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/licenses/OG-Izy
# where OG-Lapack licenses will live (license_root_dir/licenses/OG-Izy)
COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/licenses/OG-Lapack

# the OG-Maths license goes into the license_root_dir
COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/LICENSE.txt ${CMAKE_BINARY_DIR}/licenses/LICENSE.txt
# the OG-Maths licenses directory goes into the license_root_dir/licenses
COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/licenses/ ${CMAKE_BINARY_DIR}/licenses/licenses/

# the OG-Izy licenses go into (license_root_dir/licenses/OG-Izy)
COMMAND ${CMAKE_COMMAND} -E copy ${IZY_SRC_DIR}/LICENSE.txt ${CMAKE_BINARY_DIR}/licenses/OG-Izy/LICENSE.txt
COMMAND ${CMAKE_COMMAND} -E copy_directory ${IZY_SRC_DIR}/licenses/ ${CMAKE_BINARY_DIR}/licenses/OG-Izy

# the OG-Lapack licenses go into (license_root_dir/licenses/OG-Lapack)
COMMAND ${CMAKE_COMMAND} -E copy ${LAPACK_SRC_DIR}/LICENSE.txt ${CMAKE_BINARY_DIR}/licenses/OG-Lapack/LICENSE.txt
COMMENT "Creating licensing directories"
)

# make it impossible to create native libs without also creating licenses
# jar_native_libraries comes from the cache established by NativeLibraryBuilder
foreach(item ${jar_native_libraries})
  add_dependencies(${item} licensing_dirs)
endforeach()
set(jar_licenses ${licensing_dirs} CACHE INTERNAL "The jar licenses")

add_jar(${JAR_NAME_NO_EXTN} ${JAR_SOURCES}
        RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/config ${CMAKE_BINARY_DIR}/verinfo.yaml
        INCLUDE_NATIVE TRUE
        INCLUDE_LICENSES TRUE
        DEPENDS verinfo
        ENTRY_POINT com/opengamma/maths/nativeloader/NativeLibrariesSelfTest
        OUTPUT_DIR ${JARDIR})

foreach(TARGET ${TARGET_TYPES})
  if(SUPPORT_${TARGET})
    add_test(${NATIVE_SELF_TEST}_${TARGET}
             ${Java_JAVA_EXECUTABLE} -DinstructionSet=${TARGET}
             -jar ${PATH_AND_JAR_NAME})
    if (WIN32)
      string(REPLACE ";" "\\;" CLASSPATH "${CMAKE_JAVA_INCLUDE_PATH};${PATH_AND_JAR_NAME}")
    else()
      set(CLASSPATH "${CMAKE_JAVA_INCLUDE_PATH}:${PATH_AND_JAR_NAME}")
    endif()
    add_test(${NATIVE_SELF_TEST}_${TARGET}_SLF4J
             ${Java_JAVA_EXECUTABLE} -DinstructionSet=${TARGET}
             -cp ${CLASSPATH}
             com.opengamma.maths.nativeloader.NativeLibrariesSelfTest)
  endif()
endforeach()

add_test(${NATIVE_SELF_TEST} ${Java_JAVA_EXECUTABLE} -jar ${PATH_AND_JAR_NAME})

# Build source JAR
add_custom_target(sources ALL
                  COMMAND ${Java_JAR_EXECUTABLE} -cf ${PATH_AND_JAR_SOURCES_NAME}
                          com
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/java)

set(JPACKAGEFQN "com.opengamma.maths.datacontainers")
set(JAVAH_CLASSES OGNumeric)

set(JNIDIR ${CMAKE_BINARY_DIR}/src/nativeheaders)
foreach(JHCLASS ${JAVAH_CLASSES})
  jni_header_target(${JPACKAGEFQN} ${JHCLASS} ${JAR_NAME} ${JARDIR} ${JNIDIR})
endforeach()

set(JPACKAGEFQN "com.opengamma.maths.datacontainers.matrix")
set(JAVAH_CLASSES OGComplexDiagonalMatrix)

foreach(JHCLASS ${JAVAH_CLASSES})
  jni_header_target(${JPACKAGEFQN} ${JHCLASS} ${JAR_NAME} ${JARDIR} ${JNIDIR})
endforeach()

create_javadoc_jar(${JAR_NAME_NO_EXTN}
                   PACKAGES com.opengamma.maths
                   SOURCEPATH "${CMAKE_CURRENT_SOURCE_DIR}/java"
                   WINDOWTITLE "OG-Maths"
                   DOCTITLE "<h1>OG-Maths</h1>"
                   AUTHOR TRUE
                   USE TRUE
                   VERSION TRUE)

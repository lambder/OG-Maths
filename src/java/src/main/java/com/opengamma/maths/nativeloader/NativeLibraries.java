/**
 * Copyright (C) 2013 - present by OpenGamma Inc. and the OpenGamma group of companies
 *
 * Please see distribution for license.
 */
package com.opengamma.maths.nativeloader;

import static java.nio.file.Files.createTempDirectory;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URL;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map.Entry;
import java.util.Properties;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import com.opengamma.maths.exceptions.MathsExceptionOnInitialization;
import com.opengamma.maths.exceptions.MathsExceptionUnsupportedPlatform;
import com.opengamma.maths.logging.Logger;

/**
 * Used for extracting and loading native libraries.
 *
 * Default behaviour is to extract and load libraries from the JAR file.
 * Optionally, the location of the config file that specifies the libraries to be loaded can
 * be given on the command line as -Dconfigfileloc="location/file" in which case the
 * libraries are loaded from system specific library path based locations.
 *
 * The correct libraries are extracted and loaded depending on the architecture
 * that we are running on. The instruction set used in the hardware is queried as part of the
 * initialisation process so that libraries closest to the maximum level of instruction support
 * are loaded. This also be specified manually with -DinstructionSet={One of the keys in {@code instrSets}}.
 */
public final class NativeLibraries {

  /**
   *  Location to which the native libraries are extracted
   */
  private static String s_tmpDir;
  /**
   *  We initialize once only. Are we already initialized?
   *  This is guaranteed through 'synchronize'd methods
   */
  private static boolean s_initialized;

  /**
   * Store for the libraries to extract()
   */
  private static List<String> s_libsToExtract = new ArrayList<String>();

  /**
   * Store for the libraries to load()
   */
  private static List<String> s_libsToLoad = new ArrayList<String>();

  /**
   * Store for the initialisation libraries that will be extract()ed and load()ed first and separately to all others
   */
  private static List<String> s_libsForInitialise = new ArrayList<String>();

  /**
   * Properties file location, defaults to jar, can be overridden on command line
   * To do this set -DconfigFile=XXX where XXX is the location of the properties file
   */
  private static String s_configFileLocation = "/config/NativeLibraries.properties";

  /**
   * True if the properties file was specified on the command line.
   */
  private static boolean s_configFileOnCommandline;

  /**
   * Holds the tagline for the instruction set used in computation. See {@link SupportedInstructionSet}.
   */
  private static SupportedInstructionSet s_instrSet;

  /**
   * True if the instruction set was specified on the command line.
   */
  private static boolean s_instrSetOnCommandline;

  /**
   * Holds the properties associated with the native libraries.
   */
  private static Properties s_nativeLibrariesProperties = new Properties();

  /**
   * Holds the map between command line requested instruction sets and the enum (this is so the builders are happy).
   */
  private static HashMap<String, SupportedInstructionSet> instrSets;
  static {
    instrSets = new HashMap<String, SupportedInstructionSet>();
    instrSets.put("dbg", SupportedInstructionSet.DEBUG);
    instrSets.put("std", SupportedInstructionSet.STANDARD);
    instrSets.put("sse41", SupportedInstructionSet.SSE41);
    instrSets.put("sse42", SupportedInstructionSet.SSE42);
    instrSets.put("avx1", SupportedInstructionSet.AVX1);
    instrSets.put("avx2", SupportedInstructionSet.AVX2);
  }

  /**
   * Logger instance
   */
  private static Logger s_log = new Logger(NativeLibraries.class);

  /**
   * Probes the CPU and gets the maximum supported instruction set.
   * This function is implemented in the jinitialise library, which is extracted and loaded first.
   * @return maximum supported instruction set.
   */
  public static native SupportedInstructionSet getSupportedInstructionSet();

  /**
   * Prepare for loading of native libraries.
   *
   * Alg goes a bit like this:
   * *) Check if the native code is already initialised, if so do nothing.
   * *) Check if the platform is supported (detects windows, osx, linux).
   * *) Get the configuration of native libraries as specified in the properties file into a class member.
   * *) Check if a configuration file was specified on the command line, set s_configFileOnCommandline=TRUE
   * *) if(!s_configFileOnCommandline) Create a temporary folder to hold the extracted libraries.
   * *) From the properties configuration look for libraries which are slated for initalisation phase
   * *) if(!s_configFileOnCommandline) Extract libraries listed in the initialisation phase.
   * *) Load libraries listed in the initialisation phase.
   * *) If an instruction set was specified on the command line with -DinstructionSet={One of the keys in {@code instrSets}} use it,
   *    else use the now loaded jinitialise library to poke the CPUID and get the maximum supported instruction set.
   * *) if(!s_configFileOnCommandline) extract libraries listed in the "[INSTRUCTIONSET].libraries" part of the config file.
   * *) load libraries list in the "[INSTRUCTIONSET].load" part of the config file.
   *
   */
  public static synchronized void initialize() {

    if (s_initialized) {
      s_log.info("Skipping initialization - already initialized");
      return;
    }

    s_log.info("Initializing native libraries.");

    // is the platform supported?
    checkPlatformSupported();

    // get the properties file into memory
    getConfigFromProperties();

    s_libsForInitialise = getValuesFromPropertiesFileAssociatedWithString("initialise");

    Path unzipdir = null;
    if (!s_configFileOnCommandline) {
      unzipdir = createExtractionDirectory();
      extract(unzipdir, s_libsForInitialise);
    } // end if(!s_commandlineconfig)

    // load initialisation libraries
    for (String name : s_libsForInitialise) {
      load(name);
    }

    // instruction set was specified on command line
    SupportedInstructionSet instructionSet;
    if (s_instrSetOnCommandline) { // we know its valid, check was performed in getConfigFromProperties()
      instructionSet = s_instrSet;
    } else {
      instructionSet = getSupportedInstructionSet();
      s_log.info("Probed instruction set is: " + instructionSet.toString());
    }

    s_log.info("Running with instruction set as: " + instructionSet.toString());

    s_libsToExtract = getValuesFromPropertiesFileAssociatedWithString(instructionSet.getTagline() + ".libraries");
    s_log.info("Attempting to extract: " + s_libsToExtract.toString());

    if (!s_configFileOnCommandline) {
      extract(unzipdir, s_libsToExtract);
    } // end if(!s_commandlineconfig)

    // load the libraries that do the heavy lifting
    s_libsToLoad = getValuesFromPropertiesFileAssociatedWithString(instructionSet.getTagline() + ".load");
    for (String name : s_libsToLoad) {
      load(name);
    }
    s_initialized = true;
  }

  /**
   * Get the architecture's bit width
   *
   * @return the width in bits, either 32 or 64
   */
  private static int getWidth() {
    String arch = System.getProperty("os.arch");
    if (arch.equals("amd64") || arch.equals("x86_64")) {
      return 64;
    } else {
      return 32;
    }
  }

  /**
   * Check whether the current platform is supported.
   *
   * We support Mac OS X, Windows, and Linux running on x86 64-bit architectures.
   */
  private static void checkPlatformSupported() {
    List<String> supportedPlatforms = new ArrayList<>();
    supportedPlatforms.add("lnx");
    supportedPlatforms.add("w64");
    supportedPlatforms.add("w32");
    supportedPlatforms.add("osx");
    String platform = getShortPlatform();
    if (!supportedPlatforms.contains(platform)) {
      String err = "Your platform, " + getPlatform() + ", is not supported.";
      throw new MathsExceptionUnsupportedPlatform(err);
    }
  }

  /**
    * Get configuration of native library loader from file within the JAR or if
    * overridden on the command line, from the override.
    *
    * Configuration includes the names of libraries that must be extracted,
    * and the subset of those that will be explicitly loaded.
    */
  private static void getConfigFromProperties() {

    InputStream propsFile = null;

    // Is the instructionSet overridden?
    String commandLineInstrSet = System.getProperty("instructionSet");
    if (commandLineInstrSet != null) { // specified instruction set given
      s_instrSetOnCommandline = true;
      s_instrSet = instrSets.get(commandLineInstrSet);
      if (s_instrSet == null) {
        throw new MathsExceptionOnInitialization("Invalid instruction set specified on command line (value given was): "
            + commandLineInstrSet);
      }
    }

    // Is the config file location overridden?
    String commandLineConfig = System.getProperty("configFile");

    if (commandLineConfig != null) { // yes we are using a config file specified on the command line
      s_configFileOnCommandline = true;
      s_log.info("Using command line supplied configfileloc information");
      s_configFileLocation = commandLineConfig;
      s_log.info("Attempting load of config from " + s_configFileLocation);
      try {
        propsFile = new FileInputStream(s_configFileLocation);
      } catch (IOException ex) {
        throw new MathsExceptionOnInitialization("Cannot find NativeLibraries properties file in command line specified location: " + s_configFileLocation);
      }
    } else { // we are using the default config file location in the JAR
      s_log.info("Using embedded config from JAR");
      propsFile = NativeLibraries.class.getResourceAsStream(s_configFileLocation);
      if (propsFile == null) {
        throw new MathsExceptionOnInitialization("Cannot find NativeLibraries properties file in location: " + s_configFileLocation);
      }
    }
    try {
      s_nativeLibrariesProperties.load(propsFile);
    } catch (IOException e) {
      throw new MathsExceptionOnInitialization("Cannot load native library properties.");
    }
    try {
      propsFile.close();
    } catch (IOException e) {
      throw new MathsExceptionOnInitialization("Failed to close properties file stream.");
    }
  }

  /**
   * Get the current platform name.
   * @return the platform
   */
  private static String getPlatform() {
    String osName = System.getProperty("os.name");
    return osName + " " + getWidth() + "-bit";
  }

  /**
   * Return the lower-case first three letters of the platform name.
   *
   * These will match up with the names referred to in the NativeLibraries.properties file
   */
  private static String getShortPlatform() {
    String shortPlatform =  getPlatform().toLowerCase().substring(0, 3);
    String ret = null;
    if(shortPlatform.equals("lin")) {
      ret = "lnx";
    } else if(shortPlatform.equals("mac")) {
      ret = "osx";
    } else if(shortPlatform.equals("win")) {
      if (getWidth() == 32) {
        ret = "w32";
      } else if (getWidth() == 64) {
        ret = "w64";
      } else {
        throw new MathsExceptionOnInitialization("Unsupported word size found: " + getWidth());
      }
    } else {
      throw new MathsExceptionOnInitialization("Unsupported platform found: " + shortPlatform);
    }
    return ret;
  }

  /**
   * Gets values associated with "NativeLibraries.<platform_name>.propstring"
   * @param propstring the string for which to search
   * @return a list of strings associated with the property {@code propstring}
   */
  private static List<String> getValuesFromPropertiesFileAssociatedWithString(final String propstring) {
    // look for NativeLibraries specific entries
    List<String> tmplist = new ArrayList<String>();
    String nlString = "NativeLibraries." + getShortPlatform() + ".*";
    Pattern nlPattern = Pattern.compile(nlString, Pattern.CASE_INSENSITIVE);
    for (Entry<Object, Object> entry : s_nativeLibrariesProperties.entrySet()) {
      String key = entry.getKey().toString();
      Matcher nlMatch = nlPattern.matcher(key);
      if (nlMatch.matches()) {
        if (key.endsWith(propstring)) {
          String value = (String) entry.getValue();
          String[] libs = value.split(",");
          for (String lib : libs) {
            tmplist.add(lib.trim());
          }
        }
      }
    }
    return tmplist;
  }

  /**
   * Creates the directory to which og-maths is extracted
   * @return the directory to which og-maths is extracted
   */
  private static Path createExtractionDirectory() {
    Path libDir;
    try {
      libDir = createTempDirectory("og-maths-");
      libDir.toFile().deleteOnExit();
    } catch (IOException e) {
      throw new MathsExceptionOnInitialization("Could not create temp directory for native libaries", e);
    }
    return libDir;
  }

  /**
   * Extracts the libs specified to the location specified
   * @param libDir the location to which the libraries will be extracted
   * @param libsToExtract the libraries to extract
   */
  private static void extract(final Path libDir, final List<String> libsToExtract) {
    s_tmpDir = libDir.toString();

    String url = "/lib";

    Class<?> c = NativeLibraries.class;
    for (String name : libsToExtract) {
      s_log.info("Extracting " + name);
      String fsPath = libDir + "/" + name;
      String jarPath = url + "/" + getShortPlatform() + "/" + name;

      URL jarURL = c.getResource(jarPath);
      if (jarURL == null) {
        throw new MathsExceptionOnInitialization("Resource " + jarPath + " not found in jar file");
      }

      InputStream source = null;
      OutputStream destination = null;
      try {
        source = c.getResourceAsStream(jarPath);
        destination = new BufferedOutputStream(new FileOutputStream(fsPath));
        byte[] buffer = new byte[1024 * 1024];
        int len;
        boolean haveReadBytes = false;
        while ((len = source.read(buffer)) != -1) {
          if (!haveReadBytes && (len == 0)) {
            throw new MathsExceptionOnInitialization("0 bytes read for resource" + name);
          }
          haveReadBytes = true;
          destination.write(buffer, 0, len);
        }
      } catch (Exception e) {
        throw new MathsExceptionOnInitialization("Error extracting resource " + name, e);
      } finally {
        if (source != null) {
          try {
            source.close();
          } catch (IOException e) {
            throw new MathsExceptionOnInitialization("Error closing source stream for " + name, e);
          }
        }
        if (destination != null) {
          try {
            destination.close();
          } catch (IOException e) {
            throw new MathsExceptionOnInitialization("Error closing destination stream for " + name, e);
          }
        }
      }

      File file = new File(fsPath);

      try {
        file.deleteOnExit();
      } catch (SecurityException e) {
        throw new MathsExceptionOnInitialization("Security settings prevent deletion of native libraries on exit", e);
      }
    }
    s_log.info("Native libraries extracted");
  }

  /**
   * Load a native library.
   *
   * Native libraries must be extracted before they are loaded.
   *
   * @param lib the name of the library to be loaded
   */
  private static synchronized void load(final String lib) {

    if (!s_configFileOnCommandline) { // we are using the config from the file in the jar
      String libPath = s_tmpDir + File.separatorChar + lib;
      try {
        s_log.info("Loading " + lib + " from " + libPath);
        System.load(libPath);
      } catch (Exception e) {
        throw new MathsExceptionOnInitialization("Cannot load " + lib + " as " + libPath, e);
      }
    } else { // we are using a command line specified config location
      Pattern pat = Pattern.compile("lib(\\w+)..*"); // compile pattern to strip lib and .* from library name
      Matcher m = pat.matcher(lib);
      if (!m.matches()) {
        throw new MathsExceptionOnInitialization("Could not extract library name from lib string.");
      }
      String libsimplename = m.group(1); // single group
      if (libsimplename == null) {
        throw new MathsExceptionOnInitialization("Could not extract system invariant library name from library name string.");
      }
      try {
        s_log.info("Loading " + libsimplename + " from native library locations.");
        System.loadLibrary(libsimplename);
      } catch (Exception e) {
        throw new MathsExceptionOnInitialization("Cannot load " + libsimplename + " from native library locations", e);
      }

    }

  }

  private NativeLibraries() {
  };

}

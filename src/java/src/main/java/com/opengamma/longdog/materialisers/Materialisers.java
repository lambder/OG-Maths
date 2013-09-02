/**
 * Copyright (C) 2013 - present by OpenGamma Inc. and the OpenGamma group of companies
 *
 * Please see distribution for license.
 */

package com.opengamma.longdog.materialisers;

import com.opengamma.longdog.datacontainers.OGNumeric;
import com.opengamma.longdog.datacontainers.lazy.OGExpr;
import com.opengamma.longdog.datacontainers.matrix.OGArray;
import com.opengamma.longdog.nativeloader.NativeLibraries;

/**
 * 
 */
public class Materialisers {

  static {
      NativeLibraries.initialize();
  }

  private static native OGNumeric materialise(OGNumeric arg0);

  public static double[][] toJDoubleArray(OGNumeric arg0) {
    System.out.println("tree walking");
    StringBuffer buf = new StringBuffer();
    printTree(arg0, buf, 0);
    System.out.println(buf.toString());
    System.out.println("tree walking done");
    materialise(arg0);
    return new double[1][1];
  }

  public static double[][] toJComplexArray(OGNumeric arg0) {
    OGNumeric tmp = materialise(arg0);
    return null;
  }

  public static boolean toJBoolean(OGNumeric arg0) {
    OGNumeric tmp = materialise(arg0);
    return false;
  }

  public static void printTree(OGNumeric arg, StringBuffer buf, int level) {
    String tab = "   ";
    level++;
    while (!(OGArray.class.isAssignableFrom(arg.getClass()))) {
      OGExpr expr = (OGExpr) arg;
      buf.append(new String(new char[level]).replace("\0", tab) + arg.getClass() + "\n");
      for (int i = 0; i < expr.getExprs().length; i++) {
        printTree(expr.getExprs()[i], buf, level);
      }
      break;
    }
    if ((OGArray.class.isAssignableFrom(arg.getClass()))) {
      buf.append(new String(new char[level]).replace("\0", tab) + arg.getClass() + "\n");
    }
    level--;
  }

}

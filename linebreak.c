/*
 Based on code that is part of LineBreak 2.8.
 
 LineBreak is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 LineBreak is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with LineBreak; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 
 Copyright (c) 2001-2008 Josh Aas.
 Copyright (c) 2008 Alexei Svitkine.
*/

#include <stdio.h>
#include <string.h>

enum {
  jaUnixLBFormat = 0,
  jaMacLBFormat = 1,
  jaDOSLBFormat = 2
};

static void reportConvertFailureForFile(const char *filePath)
{
  printf("Failed to convert file at path: %s\n", filePath);
}

static void convertFileToUnix(const char *passedFile)
{
  FILE *handle = fopen(passedFile, "r");
  if (!handle) {
    reportConvertFailureForFile(passedFile);
    return;
  }
  FILE *tmpHandle = tmpfile();
  if (!tmpHandle) {
    fclose(handle);
    reportConvertFailureForFile(passedFile);
    return;
  }

  int curChar;
  while ((curChar = fgetc(handle)) != EOF) {
    if (curChar == '\r') {
      curChar = fgetc(handle);
      if (curChar != '\n') {
        ungetc(curChar, handle);
      }
      fputc('\n', tmpHandle);
    }
    else {
      fputc(curChar, tmpHandle);
    }
  }
  fclose(handle);

  handle = fopen(passedFile, "w");
  if (!handle) {
    fclose(tmpHandle);
    reportConvertFailureForFile(passedFile);
    return;
  }

  rewind(tmpHandle);
  while ((curChar = fgetc(tmpHandle)) != EOF) {
    fputc(curChar, handle);
  }

  fclose(handle);
  fclose(tmpHandle);
}

static void convertFileToMac(const char *passedFile)
{
  FILE *handle = fopen(passedFile, "r");
  if (!handle) {
    reportConvertFailureForFile(passedFile);
    return;
  }
  FILE *tmpHandle = tmpfile();
  if (!tmpHandle) {
    fclose(handle);
    reportConvertFailureForFile(passedFile);
    return;
  }

  int curChar;
  while ((curChar = fgetc(handle)) != EOF) {
    if (curChar == '\r') {
      curChar = fgetc(handle);
      if (curChar != '\n') {
        ungetc(curChar, handle);
      }
      fputc('\r', tmpHandle);
    }
    else if (curChar == '\n') {
      fputc('\r', tmpHandle);
    }
    else {
      fputc(curChar, tmpHandle);
    }
  }
  fclose(handle);

  handle = fopen(passedFile, "w");
  if (!handle) {
    fclose(tmpHandle);
    reportConvertFailureForFile(passedFile);
    return;
  }

  rewind(tmpHandle);
  while ((curChar = fgetc(tmpHandle)) != EOF) {
    fputc(curChar, handle);
  }

  fclose(handle);
  fclose(tmpHandle);
}

static void convertFileToDOS(const char *passedFile)
{
  FILE *handle = fopen(passedFile, "r");
  if (!handle) {
    reportConvertFailureForFile(passedFile);
    return;
  }
  FILE *tmpHandle = tmpfile();
  if (!tmpHandle) {
    fclose(handle);
    reportConvertFailureForFile(passedFile);
    return;
  }

  int curChar;
  while ((curChar = fgetc(handle)) != EOF) {
    if (curChar == '\n') {
      fputc('\r', tmpHandle);
      fputc('\n', tmpHandle);
    }
    else if (curChar == '\r') {
      curChar = fgetc(handle);
      if (curChar != '\n') {
        ungetc(curChar, handle);
      }
      fputc('\r', tmpHandle);
      fputc('\n', tmpHandle);
    }
    else {
      fputc(curChar, tmpHandle);
    }
  }
  fclose(handle);

  handle = fopen(passedFile, "w");
  if (!handle) {
    fclose(tmpHandle);
    reportConvertFailureForFile(passedFile);
    return;
  }

  rewind(tmpHandle);
  while ((curChar = fgetc(tmpHandle)) != EOF) {
    fputc(curChar, handle);
  }

  fclose(handle);
  fclose(tmpHandle);
}

static int detectLBFormat(const char *passedFile)
{
  FILE *handle = fopen(passedFile, "r");
  if (!handle) {
    printf("Failed to detect line break format, can't open file at path: %s\n", passedFile);
    return -1;
  }

  // default is UNIX
  int returnValue = jaUnixLBFormat;
  int curChar;
  while ((curChar = fgetc(handle)) != EOF) {
    if (curChar == '\r') {
      // see if the next character is '\n'
      if ((curChar = fgetc(handle)) != EOF) {
        if (curChar == '\n') {
          returnValue = jaDOSLBFormat;
          break;
        }
      }
      // if the next char isn't '\n' then this is a Mac file
      returnValue = jaMacLBFormat;
      break;
    }
    if (curChar == '\n') {
      // its UNIX, and thats the default so just break
      break;
    }
  }
  fclose(handle);
  return returnValue;
}

int main(int argc, char *argv[])
{
  int i;
  char *formats[3] = { "Unix", "Mac", "DOS" };

  if (argc >= 3) {
    if (!strcmp(argv[1], "detect")) {
      for (i = 2; i < argc; i++) {
        int format = detectLBFormat(argv[i]);
        if (format >= 0) {
          printf("Format of %s was detected to be: %s\n", argv[i], formats[format]);
        }
      }
      return 0;
    } else if (!strcmp(argv[1], "dos")) {
      for (i = 2; i < argc; i++)
        convertFileToDOS(argv[i]);
      return 0;
    } else if (!strcmp(argv[1], "mac")) {
      for (i = 2; i < argc; i++)
        convertFileToMac(argv[i]);
      return 0;
    } else if (!strcmp(argv[1], "unix")) {
      for (i = 2; i < argc; i++)
        convertFileToUnix(argv[i]);
      return 0;
    }
  } 

  printf("usage: %s [ detect | dos | mac | unix ] <file>\n", argv[0]);
  return 0;
}

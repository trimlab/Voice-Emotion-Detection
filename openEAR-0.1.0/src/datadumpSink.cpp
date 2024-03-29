/*F******************************************************************************
 *
 * openSMILE - open Speech and Music Interpretation by Large-space Extraction
 *       the open-source Munich Audio Feature Extraction Toolkit
 * Copyright (C) 2008-2009  Florian Eyben, Martin Woellmer, Bjoern Schuller
 *
 *
 * Institute for Human-Machine Communication
 * Technische Universitaet Muenchen (TUM)
 * D-80333 Munich, Germany
 *
 *
 * If you use openSMILE or any code from openSMILE in your research work,
 * you are kindly asked to acknowledge the use of openSMILE in your publications.
 * See the file CITING.txt for details.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ******************************************************************************E*/


/*  openSMILE component: datadumpSink

dump data in raw binary format (float)
the data can easily be loaded into matlab

The first float value will contain the vecsize
The second float value will contain the number of vectors
Then the matrix data follows float by float

*/



#include <datadumpSink.hpp>

#define MODULE "cDatadumpSink"

/*Library:
sComponentInfo * registerMe(cConfigManager *_confman) {
  cDataSink::registerComponent(_confman);
}
*/

SMILECOMPONENT_STATICS(cDatadumpSink)

//sComponentInfo * cDatadumpSink::registerComponent(cConfigManager *_confman)
SMILECOMPONENT_REGCOMP(cDatadumpSink)
{
  SMILECOMPONENT_REGCOMP_INIT
  scname = COMPONENT_NAME_CDATADUMPSINK;
  sdescription = COMPONENT_DESCRIPTION_CDATADUMPSINK;

  // we inherit cDataSink configType and extend it:
  SMILECOMPONENT_INHERIT_CONFIGTYPE("cDataSink")

  SMILECOMPONENT_IFNOTREGAGAIN(
    ct->setField("filename","binary file to write to","datadump.dat");
    ct->setField("lag","output data <lag> frames behind",0);
    ct->setField("append","append to existing file (1/0 = yes/no)",0);
  )

  SMILECOMPONENT_MAKEINFO(cDatadumpSink);
}

SMILECOMPONENT_CREATE(cDatadumpSink)

//-----

cDatadumpSink::cDatadumpSink(const char *_name) :
  cDataSink(_name),
  filehandle(NULL),
  filename(NULL),
  nVec(0),
  vecSize(0)
{
}

void cDatadumpSink::fetchConfig()
{
  cDataSink::fetchConfig();
  
  filename = getStr("filename");
  SMILE_DBG(3,"filename = '%s'",filename);

  lag = getInt("lag");
  SMILE_DBG(3,"lag = %i",lag);

  append = getInt("append");
  if (append) SMILE_DBG(3,"append to file is enabled");
}

/*
int cDatadumpSink::myConfigureInstance()
{
  int ret=1;
  ret *= cDataSink::myConfigureInstance();
  // ....
  //return ret;
}
*/


int cDatadumpSink::myFinaliseInstance()
{
  int ap=0;
  float tmp=0;
  
  int ret = cDataSink::myFinaliseInstance();
  if (ret==0) return 0;
  
  if (append) {
    // check if file exists:
    filehandle = fopen(filename, "rb");
    if (filehandle != NULL) {
      // load vecsize, to see if it matches!
      if (fread(&tmp,sizeof(float),1,filehandle)) vecSize=(long)tmp;
      else vecSize = 0;
      // load initial nVec
      if (fread(&tmp,sizeof(float),1,filehandle)) nVec=(long)tmp;
      else nVec = 0;
      fclose(filehandle);
      filehandle = fopen(filename, "ab");
      ap=1;
    } else {
      filehandle = fopen(filename, "wb");
    }
  } else {
    filehandle = fopen(filename, "wb");
  }
  if (filehandle == NULL) {
    COMP_ERR("Error opening binary file '%s' for writing (component instance '%s', type '%s')",filename, getInstName(), getTypeName());
  }
  
  if (vecSize == 0) vecSize = reader->getLevelN();

  if (!ap) {
    // write mini dummy header ....
    writeHeader();
  }
  
  return ret;
}


int cDatadumpSink::myTick(long long t)
{
  if (filehandle == NULL) return 0;
  
  SMILE_DBG(4,"tick # %i, writing value vector (lag=%i):",t,lag);
  cVector *vec= reader->getFrameRel(lag);  //new cVector(nValues+1);
  if (vec == NULL) return 0;

  // now print the vector:
  int i; float *tmp = (float*)malloc(sizeof(float)*vec->N);
  if (tmp==NULL) OUT_OF_MEMORY;
  
  if (vec->type == DMEM_FLOAT) {
    for (i=0; i<vec->N; i++) {
      tmp[i] = (float)(vec->dataF[i]);
    }
  } else if (vec->type == DMEM_INT) {
    for (i=0; i<vec->N; i++) {
      tmp[i] = (float)(vec->dataI[i]);
    }
  } else {
    SMILE_ERR(1,"unknown data type %i",vec->type);
    return 0;
  }

  int ret=1;
  if (!fwrite(tmp,sizeof(float),vec->N,filehandle)) {
    SMILE_ERR(1,"Error writing to raw feature file '%s'!",filename);
    ret = 0;
  } else {
    //reader->nextFrame();
    nVec++;
  }

  free(tmp);

  // tick success
  return ret;
}

// WARNING: write header changes file write pointer to beginning of file (after header)
void cDatadumpSink::writeHeader()
{
  // seek to beginning of file:
  fseek( filehandle, 0, SEEK_SET );
  // write header:
  float tmp;
  tmp = (float)vecSize;
  fwrite(&tmp, sizeof(float), 1, filehandle);
  tmp = (float)nVec;
  fwrite(&tmp, sizeof(float), 1, filehandle);
}

cDatadumpSink::~cDatadumpSink()
{
  // write final header 
  writeHeader();
  // close output file
  fclose(filehandle);
}


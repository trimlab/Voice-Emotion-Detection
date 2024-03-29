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


/*  openSMILE component:

TODO:
Average Magnitude Difference Function (AMDF)

this implements a simple windowed AMDF


(further TODO: continuous AMDF of fixed delay and decay -> WindowProcessor)
*/


#include <amdf.hpp>

#define MODULE "cAmdf"

/*Library:
sComponentInfo * registerMe(cConfigManager *_confman) {
cDataProcessor::registerComponent(_confman);
}
*/

SMILECOMPONENT_STATICS(cAmdf)

SMILECOMPONENT_REGCOMP(cAmdf)
//sComponentInfo * cAmdf::registerComponent(cConfigManager *_confman)
{
  SMILECOMPONENT_REGCOMP_INIT
    scname = COMPONENT_NAME_CAMDF;
  sdescription = COMPONENT_DESCRIPTION_CAMDF;

  // we inherit cVectorProcessor configType and extend it:
  SMILECOMPONENT_INHERIT_CONFIGTYPE("cVectorProcessor")

    SMILECOMPONENT_IFNOTREGAGAIN(
    ct->setField("nameAppend",NULL,"amdf");
  ct->setField("nLag","compute AMDF up to maximum lag nLag (or maximum possible lag limited by framesize), (if nLag=0, always the maximum framesize will be used <- this is the default. if nLag<0 nLag=framesize/((-1)*nLag) will be used)",0);
  ct->setField("method","AMDF computation method (i.e. handling of border values):\n    limit :  compute AMDF only in regions where the shifted windows overlap\n    warp : compute cyclical AMDF by warping\n    zeropad : zero pad missing samples","limit");
  ct->setField("invert","invert AMDF output values (i.e. so that it corresponds more to an autocorrelation function)",0);

  )
    SMILECOMPONENT_MAKEINFO(cAmdf);
}

SMILECOMPONENT_CREATE(cAmdf)

//-----

cAmdf::cAmdf(const char *_name) :
cVectorProcessor(_name),
nLag(1),
method(AMDF_LIMIT),
invert(0)
{

}

void cAmdf::fetchConfig()
{
  cVectorProcessor::fetchConfig();

  const char *_method = getStr("method");
  if (_method != NULL) {
    if (!strcmp(_method,"limit")) {
      method = AMDF_LIMIT;
    } else
      if (!strcmp(_method,"warp")) {
        method = AMDF_WARP;
      } else
        if (!strcmp(_method,"zeropad")) {
          method = AMDF_ZEROPAD;
        }
  }
  nLag = getInt("nLag");
  invert = getInt("invert");
}

/*
int cAmdf::myConfigureInstance()
{
int ret=1;
ret *= cVectorProcessor::myConfigureInstance();
if (ret == 0) return 0;

//...


return ret;
}
*/

/*
int cAmdf::configureWriter(const sDmLevelConfig *c)
{

// you must return 1, in order to indicate configure success (0 indicated failure)
return 1;
}

*/


int cAmdf::setupNamesForField(int i, const char*name, long nEl)
{
  if (nLag == 0) nLag = nEl-1;
  if (nLag < 0) nLag = (nEl)/((-1)*nLag) - 1;
  if (nLag < 1) nLag = 1;
  if (nLag > nEl-1) nLag = nEl-1;
  SMILE_DBG(2,"nLag = %i",nLag);
  return cVectorProcessor::setupNamesForField(i,name,nLag);
}


/*
int cAmdf::myFinaliseInstance()
{
int ret=1;
ret *= cVectorProcessor::myFinaliseInstance();
//.....
return ret;
}
*/

/*
// a derived class should override this method, in order to implement the actual processing
int cAmdf::processVectorInt(const INT_DMEM *src, INT_DMEM *dst, long Nsrc, long Ndst, int idxi) // idxi=input field index
{
// do domething to data in *src, save result to *dst
// NOTE: *src and *dst may be the same...

return 0;
}
*/

// SMILElib function:
// compute AMDF
int computeAMDF(const FLOAT_DMEM *in, FLOAT_DMEM *out, long nIn, long nOut)
{ // assumption: nOut = nLag!
  long j; // lag j
  FLOAT_DMEM tmp;
  const FLOAT_DMEM *in1, *in2, *end;
  *(out++) = 0.0;
  for (j=1; j<nOut; j++) {
    tmp=0.0;
    in1 = in;
    end = in+nIn-j;
    in2 = in+j;
    while (in1<end) {
      tmp += fabs( *(in1++) - *(in2++) );
    }
    *(out++) = (tmp)/(FLOAT_DMEM)(nIn-j);
  }
  return 1;
}

// SMILElib function:
// compute warped AMDF from a vector
int computeAMDFwarped(const FLOAT_DMEM *in, FLOAT_DMEM *out, long nIn, long nOut, int invert=0)
{ // assumption: nOut = nLag!
  long j; // lag j
  FLOAT_DMEM tmp;
  const FLOAT_DMEM *in1, *in2, *end;
  *(out) = 0.0;

  for (j=1; j<nOut; j++) {
    //printf("j=%i\n",j);
    tmp=0.0;
    in1 = in;
    end = in+nIn;
    in2 = in+j;
    //long tt = 0;
    while (in2<end) {
      //while (tt<nIn-j) {
      tmp += fabs( *(in1++) - *(in2++) );
      //	tt++;
    }
    in2 = in;

    while (in1<end) {
      //while (tt<nIn) {
      tmp += fabs( *(in1++) - *(in2++) );
      //tt++;
    }
    out[j] = (tmp)/(FLOAT_DMEM)(nIn);
  }
  return 1;
}

// SMILElib function:
// compute zeropadded AMDF from a vector
int computeAMDFzeropad(const FLOAT_DMEM *in, FLOAT_DMEM *out, long nIn, long nOut)
{ // assumption: nOut = nLag!
  long j; // lag j
  FLOAT_DMEM tmp;
  const FLOAT_DMEM *in1, *in2, *end1, *end2;
  *(out++) = 0.0;
  for (j=1; j<nOut; j++) {
    tmp=0.0;
    in1 = in;
    end1 = in+nIn-j;
    end2 = in+nIn;
    in2 = in+j;
    while (in1<end1) {
      tmp += fabs( *(in1++) - *(in2++) );
    }
    while (in1<end2) { // TODO: avoid double computation effort here.... sum up only once and then look up cummulative value
      tmp += fabs( *(in1++) );
    }
    *(out++) = (tmp)/(FLOAT_DMEM)(nIn);
  }
  return 1;
}

// a derived class should override this method, in order to implement the actual processing
int cAmdf::processVectorFloat(const FLOAT_DMEM *src, FLOAT_DMEM *dst, long Nsrc, long Ndst, int idxi) // idxi=input field index
{
  // do domething to data in *src, save result to *dst
  // NOTE: *src and *dst may be the same...
  if (Nsrc == 0) return 0;
  printf("Enter...\n");

  switch (method) {
    case AMDF_LIMIT:  computeAMDF(src,dst,Nsrc,Ndst); break;
    case AMDF_WARP:  computeAMDFwarped(src,dst,Nsrc,Ndst, invert); break;
    case AMDF_ZEROPAD:  computeAMDFzeropad(src,dst,Nsrc,Ndst); break;
    default : {
      SMILE_ERR(1,"unknown AMDF computation method: %i",method);
      return 0;
              }
  }

  if (invert) {
    FLOAT_DMEM max = 0.0;
    long j;
    for (j=0; j<Ndst; j++)
      if (dst[j] > max ) max = dst[j];
    for (j=0; j<Ndst; j++)
      dst[j] = max - dst[j];
  }
  printf("Leave...\n");

  return 1;
}

cAmdf::~cAmdf()
{
}


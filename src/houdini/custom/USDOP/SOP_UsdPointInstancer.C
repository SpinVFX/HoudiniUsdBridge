/*
* Copyright 2021 Side Effects Software Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "SOP_UsdPointInstancer.h"
#include "SOP_UsdPointInstancer.proto.h"

#include <HUSD/HUSD_PointInstancer.h>

#include <HUSD/HUSD_DataHandle.h>
#include <HUSD/HUSD_ErrorScope.h>
#include <HUSD/HUSD_Info.h>
#include <HUSD/HUSD_TimeCode.h>

#include <PRM/PRM_TemplateBuilder.h>
#include <OP/OP_Operator.h>

#include <SOP/SOP_Error.h>
#include <SOP/SOP_Node.h>
#include <SOP/SOP_NodeVerb.h>

#include <LOP/LOP_Node.h>

#include <UT/UT_StringHolder.h>
#include <UT/UT_Regex.h>

#include <HUSD/XUSD_Data.h>
#include <LOP/LOP_PRMShared.h>


PXR_NAMESPACE_OPEN_SCOPE

static const char* theDsFile = R"THEDSFILE(
{
    name	parameters
    parm {
        name    "loppath"
        cppname "LOPPath"
        label   "LOP Path"
        type    oppath
        default { "" }
        parmtag { "opfilter" "!!LOP!!" }
        parmtag { "oprelative" "." }
    }
    parm {
        name    "primpath"
        cppname "PrimPath"
        label   "Primitive Path"
        type    string
        default { "" }
        menutoggle {
            [ "import loputils" ]
            [ "return loputils.createPrimPathMenu()" ]
            language python
        }
        parmtag { "script_action" "import loputils\nkwargs['ctrl'] = True\nloputils.selectPrimsInParm(kwargs, True,\n    lopparmname='loppath', allowinstanceproxies=True)" }
        parmtag { "script_action_help" "Select primitives using the primitive picker dialog." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
        parmtag { "sidefx::usdpathtype" "prim" }
    }
    parm {
        name    "primvarsfilter"
        cppname "PrimvarsFilter"
        label   "Primvars Filter"
        type    string
        default { "*" }
        menutoggle {
            [ "from pxr import UsdGeom" ]
            [ "" ]
            [ "menu = []" ]
            [ "node = kwargs['node']" ]
            [ "loppath = node.parm('loppath').eval()" ]
            [ "primpath = node.parm('primpath').eval()" ]
            [ "" ]
            [ "try:" ]
            [ "    stage = hou.node(loppath).stage()" ]
            [ "except:" ]
            [ "    return []" ]
            [ "" ]
            [ "if not stage:" ]
            [ "    return []" ]
            [ "" ]
            [ "api = UsdGeom.PrimvarsAPI(stage.GetPrimAtPath(primpath))" ]
            [ "if not api:" ]
            [ "    return []" ]
            [ "    " ]
            [ "for primvar in api.GetPrimvarsWithAuthoredValues():" ]
            [ "    menu.extend([primvar.GetPrimvarName(), primvar.GetPrimvarName()])" ]
            [ "" ]
            [ "return menu" ]
            language python
        }
    }
    parm {
        name    "createusdprimpath"
        cppname "CreateUsdPrimPath"
        label   "Create USD Prim Path Attribute"
        type    toggle
        default { "1" }
    }
    parm {
        name    "sepparm"
        label   "Separator"
        type    separator
        default { "" }
    }
    parm {
        name    "xformmode"
        cppname "XformMode"
        label   "Transform"
        type    string
        default { "local" }
        menu {
            "none"   "None"
            "world"  "Into World Space"
        }
    }
    parm {
        name    "importpositions"
        cppname "ImportPositions"
        label   "Import Positions"
        type    toggle
        default { "1" }
    }
    parm {
        name    "importorientations"
        cppname "ImportOrientations"
        label   "Import Orientation"
        type    toggle
        default { "1" }
    }
    parm {
        name    "importscales"
        cppname "ImportScales"
        label   "Import Scales"
        type    toggle
        default { "1" }
    }
    parm {
        name    "importaccelerations"
        cppname "ImportAccelerations"
        label   "Import Accelerations"
        type    toggle
        default { "1" }
    }
    parm {
        name    "importvelocities"
        cppname "ImportVelocities"
        label   "Import Velocities"
        type    toggle
        default { "1" }
    }
    parm {
        name    "importangularvelocities"
        cppname "ImportAngularVelocities"
        label   "Import Angular Velocities"
        type    toggle
        default { "1" }
    }
    parm {
        name    "createusdxformattrib"
        cppname "CreateUsdXformAttrib"
        label   "Create Usd Xform Attribute"
        type    toggle
        default { "1" }
        hidewhen "{ xformmode != world }"
    }
    parm {
        name    "sepparm2"
        label   "Separator"
        type    separator
        default { "" }
    }
    parm {
        name    "idmode"
        cppname "IdMode"
        label   "Create id Attr"
        type    string
        default { "fromeither" }
        menu {
            "none"        "None"
            "fromattr"    "From ids USD Attribute"
            "fromptnum"   "From Point Number"
            "fromeither"  "From ids USD Attribute or Point Number"
        }
    }
    parm {
        name    "protomode"
        cppname "ProtoMode"
        label   "Import Prototypes Attr"
        type    string
        default { "fromattr" }
        menu {
            "none"           "None"
            "fromattr"       "From ProtoIndices Attribute"
            "fromprimpath"   "From Prototype Prim Path"
        }
    }
    parm {
        name    "importinvisids"
        cppname "ImportInvisIds"
        label   "Create Invisibility Attribute"
        type    toggle
        default { "1" }
    }
    parm {
        name    "sepparm3"
        label   "Separator"
        type    separator
        default { "" }
    }
    parm {
        name    "bboxmode"
        cppname "BBoxMode"
        label   "Import Bounding Box"
        type    string
        default { "none" }
        joinnext
        menu {
            "none"      "None"
            "asattr"    "As Attribute"
            "aspacked"  "As Packed Primitive"
            "asboth"    "As Attribute and Packed Primitive"
        }
    }
    parm {
        name    "boundingboxpurposes"
        cppname "BoundingBoxPurposes"
        label   "Purposes"
        type    string
        default { "default" }
        menureplace {
            "default"       "default"
            "render,proxy"  "render,proxy"
            "render"        "render"
            "proxy"         "proxy"
            "guide"         "guide"
        }
        hidewhen "{ bboxmode == none }"
    }
    parm {
        name    "bboxattr"
        cppname "BBoxAttr"
        label   "Attribute"
        type    string
        default { "bounds" }
        hidewhen "{ bboxmode != asattr bboxmode != asboth }"
    }
}
)THEDSFILE";

PRM_Template *
SOP_UsdPointInstancer::buildTemplates()
{
   static PRM_TemplateBuilder templ("SOP_UsdPointInstancer.C", theDsFile);
   return templ.templates();
}

OP_Operator *
SOP_UsdPointInstancer::createOperator()
{
   return new OP_Operator(
           "usdpointinstancerimport", "Usd Point Instancer Import", myConstructor, buildTemplates(), 0,
           0, nullptr);
}

SOP_UsdPointInstancer::SOP_UsdPointInstancer(
       OP_Network *net,
       const char *name,
       OP_Operator *op)
   : SOP_Node(net, name, op)
{
   mySopFlags.setManagesDataIDs(true);
}

OP_ERROR
SOP_UsdPointInstancer::cookMySop(OP_Context &context)
{
   return cookMyselfAsVerb(context);
}

class SOP_UsdPointInstancerVerb : public SOP_NodeVerb
{
public:
    SOP_UsdPointInstancerVerb() {}
   ~SOP_UsdPointInstancerVerb() override {}

   SOP_NodeParms *allocParms() const override
   {
       return new SOP_UsdPointInstancerParms();
   }

   UT_StringHolder name() const override
   {
       return "loppointinstancer";
   }

   CookMode cookMode(const SOP_NodeParms *parms) const override
   {
       return COOK_GENERATOR;
   }

   void cook(const CookParms &cookparms) const override;
};

static SOP_NodeVerb::Register<SOP_UsdPointInstancerVerb> theSOPLOPPointInstancerVerb;

const SOP_NodeVerb *
SOP_UsdPointInstancer::cookVerb() const
{
   return theSOPLOPPointInstancerVerb.get();
}

void
SOP_UsdPointInstancerVerb::cook(const CookParms &cookparms) const
{
    // TODO:  Attach a usdprimpath (group?) attribute in SOPs (optionally)
    //        and allow for multiple point instancers at once
    //        reverse must (optionally?) lookup primpath from attr
    //        (or put everything into one new instancer?)
    HUSD_ErrorScope errorscope(cookparms.error());
    OP_Context      context(cookparms.getContext());

    auto &&parms = cookparms.parms<SOP_UsdPointInstancerParms>();

    LOP_Node *lop = cookparms.getCwd()->getLOPNode(parms.getLOPPath());
    if (!lop)
    {
        cookparms.sopAddError(SOP_MESSAGE, "Invalid LOP node path.");
        return;
    }

    // add lop's dataMicroNode as an input to trigger a re-cook when the lop
    // data changes.
    cookparms.addExplicitInput(lop->dataMicroNode());

    UT_StringRef  primPath(parms.getPrimPath());
    HUSD_TimeCode timeCode(context.getTime(), HUSD_TimeCode::FRAME);

    GU_Detail      *gdp = cookparms.gdh().gdpNC();
    if (gdp == nullptr)
    {
        cookparms.sopAddError(SOP_MESSAGE, "Invalid GU Detail.");
        return;
    }

    HUSD_AutoReadLock   readlock(lop->getCookedDataHandle(context));
    if (!readlock.constData() ||
        !readlock.constData()->isStageValid())
    {
        cookparms.sopAddError(SOP_MESSAGE,
                              "Invalid LOP Network or USD Stage");
        return;
    }

    HUSD_Info husdinfo(readlock);
    if (!husdinfo.isPrimAtPath(primPath) ||
        !husdinfo.isPrimType(primPath, "PointInstancer"))
    {
        cookparms.sopAddError(SOP_MESSAGE,
                              "Invalid PrimPath Supplied.");
        return;
    }

    int       numinstances = husdinfo.getPointInstancerInstanceCount(primPath,
                                                               timeCode);
    GA_Offset offsetStart = gdp->appendPointBlock(numinstances);

    HUSD_PointInstancer::IdSource idSource = HUSD_PointInstancer::IDSOURCENONE;
    if (parms.getIdMode() == "fromattr")
    {
        idSource = HUSD_PointInstancer::IDSOURCEATTRIBUTE;
        if (!husdinfo.hasAuthoredValueForAttrib(primPath, "ids"))
        {
            cookparms.sopAddWarning(SOP_MESSAGE,
                                    "No 'ids' Attribute found on prim.");
        }
    }
    else if (parms.getIdMode() == "fromptnum")
    {
        idSource = HUSD_PointInstancer::IDSOURCEPOINTNUMBER;
    }
    else if (parms.getIdMode() == "fromeither")
    {
        idSource = HUSD_PointInstancer::IDSOURCEATTRIBUTEORPOINTNUMBER;
    }

    HUSD_PointInstancer::ProtoSource protoSource = HUSD_PointInstancer::PROTOSOURCENONE;
    if (parms.getProtoMode() == "fromattr")
        protoSource = HUSD_PointInstancer::PROTOSOURCEATTRIBUTE;
    else if (parms.getProtoMode() == "fromprimpath")
        protoSource = HUSD_PointInstancer::PROTOSOURCEPRIMPATH;

    HUSD_PointInstancer::copyUsdIdAttrsToGeoAttrs(readlock, gdp, offsetStart,
                                                  numinstances, timeCode,
                                                  primPath,
                                                  parms.getImportInvisIds(),
                                                  idSource,
                                                  protoSource);

    HUSD_PointInstancer::copyUsdXformAttrsToGeoAttrs(readlock, gdp, offsetStart,
                                                     numinstances, timeCode,
                                                     primPath,
                                                     parms.getXformMode() == "world",
                                                     parms.getImportPositions(),
                                                     parms.getImportOrientations(),
                                                     parms.getImportScales(),
                                                     parms.getImportAccelerations(),
                                                     parms.getImportVelocities(),
                                                     parms.getImportAngularVelocities(),
                                                     parms.getCreateUsdXformAttrib());

    HUSD_PointInstancer::copyUsdPrimvarsToGeoAttrs(readlock, gdp, offsetStart,
                                                   numinstances,
                                                   parms.getPrimvarsFilter(),
                                                   timeCode, primPath);

    if (parms.getBBoxMode() != "none")
    {
        UT_StringArray  purposes;
        UT_Regex        delimiter("[, ]+");
        delimiter.split(parms.getBoundingBoxPurposes(), purposes);
        HUSD_PointInstancer::createBoundingBoxGeoAttr(readlock, gdp,
                                                        offsetStart, numinstances,
                                                        timeCode, primPath, purposes,
                                                        parms.getXformMode() == "world");
    }

    if (parms.getCreateUsdPrimPath())
    {
        GA_RWHandleS usdprimpathhandle = gdp->getAttributes().createStringAttribute(
                        GA_ATTRIB_POINT, GA_SCOPE_PUBLIC,
                        "usdprimpath", 1);
        GA_Offset ptoff;
        for (int i = 0; i < numinstances; ++i)
        {
            ptoff = offsetStart + i;
            usdprimpathhandle.set(ptoff, primPath);
        }
    }
}

const char* SOP_UsdPointInstancer::inputLabel(OP_InputIdx idx) const
{
    UT_ASSERT(idx >= 0);
    return "";
}

PXR_NAMESPACE_CLOSE_SCOPE

#ifndef DataTypesH
#define DataTypesH
//---------------------------------------------------------------------------
/**
* @file DataTypes.h
* Here is a universal place to define model-specific data types, structures,
* enums, etc.
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/

 /**
  * Structure for holding species-specific float values. An object which does
  * not apply to all species does not need to have data for all species for
  * those data that are species-specific.  (Besides, that data may not exist
  * in the first place.)  This allows bundling of a value with species code to
  * which it applies.
  *
  * The naming convention rules are being ignored for these so that they can
  * all use the same internal names - making coding and type-changing easier.*/
 struct floatVal {float val; /**<Value.*/
                  short int code; /**<Species code.*/
                  };

 /**
  * Structure for holding species-specific integer values. An object which does
  * not apply to all species does not need to have data for all species for
  * those data that are species-specific.  (Besides, that data may not exist
  * in the first place.)  This allows bundling of a value with species code to
  * which it applies.
  *
  * The naming convention rules are being ignored for these so that they can
  * all use the same internal names - making coding and type-changing easier.*/
 struct intVal {int val; /**<Value.*/
                short int code; /**<Species code.*/
                };

 /**
  * Structure for holding species-specific double values. An object which does
  * not apply to all species does not need to have data for all species for
  * those data that are species-specific.  (Besides, that data may not exist
  * in the first place.)  This allows bundling of a value with species code to
  * which it applies.
  *
  * The naming convention rules are being ignored for these so that they can
  * all use the same internal names - making coding and type-changing easier.*/
 struct doubleVal {double val; /**<Value.*/
                short int code; /**<Species code.*/
                };

 /**
  * Structure for holding species-specific boolean values. An object which does
  * not apply to all species does not need to have data for all species for
  * those data that are species-specific.  (Besides, that data may not exist
  * in the first place.)  This allows bundling of a value with species code to
  * which it applies.
  *
  * The naming convention rules are being ignored for these so that they can
  * all use the same internal names - making coding and type-changing easier.*/
 struct boolVal {bool val; /**<Value.*/
                short int code; /**<Species code.*/
                };

 /**Holds species/type combos.*/
 struct stcSpeciesTypeCombo {
   short int iSpecies,  /**<Species number.  Matches tree population's list.*/
             iType; /**<Type number.  Matches tree population's list.*/
             };


/**List of file type codes.*/
enum fileType{
  notrecognized = -1, /**<Unrecognized file type*/
  oldsortie = 0, /**<File from old version of SORTIE*/
  parfile = 1,   /**<Parameter file*/
  tree,          /**<General tree file*/
  treemap,       /**<Tree map file only*/
  batchfile,     /**<Batch file*/
  map,           /**<Grid map*/
  detailed_output,       /**<Detailed output file*/
  detailed_output_timestep, /**<Detailed output timestep file*/
  lastfile       /**<Placeholder*/
};

/** List of PDFs. */
enum pdf {
  deterministic_pdf, /**<deterministic, no PDF*/
  poisson_pdf, /**<Poisson*/
  lognormal_pdf, /**<Lognormal*/
  normal_pdf, /**<Normal*/
  negative_binomial_pdf, /**<Negative binomial*/
  binomial_pdf, /**<Binomial*/
  inverse_gaussian_pdf /**Inverse Gaussian*/
};

/**Reason codes for why a tree died.  This controls the life history stage
  they pass to as dead trees.*/
namespace whyDead {
  enum deadCode {
    notdead = 0, /**<Tree is not dead*/
    harvest,   /**<Harvested*/
    natural,   /**<Natural causes - regular mortality functions*/
    disease,   /**<Killed by disease*/
    fire,      /**<Killed by fire*/
    insects,   /**<Killed by insects*/
    storm,     /**<Killed by a storm (windthrow)*/
    remove_tree /**<Request an immediate removal of this tree from memory*/
  };
}

/**Masting vs. non-masting events*/
enum mastEvent {nonmast,     /**<Non-masting event*/
                mast,  /**<Masting event*/
                numevents /**<Number of event categories*/
};


//---------------------------------------------------------------------------
#endif

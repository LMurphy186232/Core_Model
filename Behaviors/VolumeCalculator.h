//---------------------------------------------------------------------------
// VolumeCalculator
//---------------------------------------------------------------------------
#if !defined(VolumeCalculator_H)
  #define VolumeCalculator_H

#include "BehaviorBase.h"

/**
* Tree Volume Calculator Version 1.0
*
* This class calculates tree volume.  It can be set up to specifically
* calculate merchantable volume.  The volume value is stored in an float tree
* data member that this behavior adds as a value in cubic meters.
*
* Volume is calculated by dividing the tree trunk into segments.  The volume
* of each segment is calculated as:
* <center><i>V = ((A<sub>1</sub> + A<sub>2</sub>)/2) * l</i></center>
* where:
* <ul>
* <li><i>A<sub>1</sub></i> = cross-sectional area at bottom of segment, in
* square meters</li>
* <li><i>A<sub>2</sub></i> = cross-sectional area at top of segment, in square
* meters</li>
* <li><i>V</i> = volume of the section, in cubic meters</li>
* <li><i>l</i> = length of section, in meters</li>
* </ul>
*
* <i>A</i> is just the plain old area of a circle:
* @htmlonly <center><i>A = &pi;(d/2)<sup>2</sup></i></center> @endhtmlonly
*
* where <i>d</i> is the diameter of the trunk inside the bark at that point in
* the trunk.
*
* The diameter of the trunk inside the bark at a particular height on the
* trunk comes from the taper equation (the "2002 model", Kozak (2004) Forest
* Chronicle 80: 507 - 515).  (The equation below probably has different
* parameter names from the user documentation because that one uses too many
* subscripts.)  The equation is:
* <center><i>d = aD<sup>b</sup>H<sup>c</sup>X<sup>dz<sup>4</sup>+f(1/exp(D/H)) + gX<sup>0.1</sup>+i(1/D)+jH<sup>Q</sup>+kX</sup></i></center>
*
* where:
* <ul>
* <li><i>X = (1 - (h/H)<sup>1/3</sup>) / (1 - p<sup>1/3</sup>)</i></li>
* <li><i>Q = 1 - (h/H)<sup>1/3</sup></i></li>
* <li><i>p = 1.3/H</i></li>
* <li><i>z = h / H </i></li>
* <li><i>D</i> = outside bark diameter at breast height (cm), calculated from
* DBH using the MathLibrary's AddBarkToDBH() function</li>
* <li><i>H</i> = total tree height (m)</li>
* <li><i>h</i> = height from the ground to the point at which to find the
* inside bark diameter, in meters</li>
* <li><i>d</i> = inside bark diameter at <i>h</i> height from ground (cm)</li>
* <li><i>a,b,c,d,f,g,i,j,</i> and <i>k</i> are parameters</li>
* </ul>
*
* To find the total volume, the trunk needs to be defined with a starting point
* and an ending point.  For total trunk volume, the starting point is the
* ground and the ending point is the top of the tree.  However, if one is
* trying to calculate merchantable volume, then the points are different.  Both
* of these points are defined by the user.  The starting point is given by the
* user as a "stump height" in cm off the ground (set into mp_fStumpHeight).
* The ending point is the point at which the trunk has narrowed to a diameter
* too small to be useful, and is given by the user as the "minimum usable
* diameter" in cm (set into mp_fMinUsableDiam).  If both the stump height and
* minimum usable diameter are set to zero, then the total trunk volume is
* calculated.
*
* As was described above, the volume of the trunk is determined by summing up
* the volumes of individual trunk segments.  The length of these segments is
* set by the user.  Thus the user is able to find their tradeoff point between
* accuracy (shorter segments) and quick processing time (longer segments).
* The volume calculation starts at the starting point of the trunk defined in
* the "stump height" parameter.  It finds the volume of the segment defined by
* the segment length value.  Then it moves to the end of the segment, using
* it as the starting point for the next segment.  The calculation works its
* way down the tree until either 1) a segment starting or ending diameter is
* less than the minimum usable diameter or 2) the end of the tree is reached.
* A segment whose starting or ending diameter is less than the minimum usable
* is not used at all in the volume total.
*
* Note:  this behavior is VERY prone to math errors.  There are a lot of
* X^Y types of calculations, with neither X nor Y known until runtime.  There
* are so many parameters that you can't really validate them ahead of time
* either.  I can't think of anything that I can do about it.  I can't even
* figure out a way to catch math errors.  So if the user chooses badly, crashy
* crashy.
*
* This behavior adds an float data member called "Volume" to trees that holds
* the final volume calculation as a value in square meters.
*
* This class's namestring and parameter call string are both
* "TreeVolumeCalculator".
*
* This behavior may not be applied to seedlings.
*/
class clVolumeCalculator : virtual public clBehaviorBase {
  public:

  /**
  * Constructor.
  * @param p_oSimManager clSimManager object.
  */
  clVolumeCalculator(clSimManager *p_oSimManager);

  /**
  * Destructor.  Deletes arrays.
  */
  ~clVolumeCalculator();

  /**
  * Makes volume calculations.  A query is sent to the tree population to get
  * all trees to which this behavior is applied.  For each, the volume is
  * calculated by GetTreeVolume().  This value is placed in the "Volume" float
  * tree data member.
  */
  void Action();

  /**
  * Does setup for this behavior.  Reads values from the parameter file and
  * validates them.  It then calls FormatQueryString() to make our query
  * string.  Last, it calls Action() so that the initial conditions volume will
  * be added.
  * @param p_oDoc DOM tree of parsed input file.
  * @throw modelErr if:
  * <ul>
  * <li>The stump height is less than 0</li>
  * <li>The minimum usable diameter is less than 0</li>
  * <li>The segment length is less than or equal to 0</li>
  * </ul>
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
  * Registers the "Volume" float data member.  The return codes are captured
  * in the mp_iVolumeCodes array.
  * @throw modelErr if this behavior is being applied to any tree type except
  * saplings, adults, and snags.
  */
  void RegisterTreeDataMembers();

  protected:
  /** <i>a</i> in the taper equation.  Array size is # behavior species.*/
  float *mp_fTaperA;

  /** <i>b</i> in the taper equation.  Array size is # behavior species.*/
  float *mp_fTaperB;

  /** <i>c</i> in the taper equation.  Array size is # behavior species.*/
  float *mp_fTaperC;

  /** <i>d</i> in the taper equation.  Array size is # behavior species.*/
  float *mp_fTaperD;

  /** <i>f</i> in the taper equation.  Array size is # behavior species.*/
  float *mp_fTaperF;

  /** <i>g</i> in the taper equation.  Array size is # behavior species.*/
  float *mp_fTaperG;

  /** <i>i</i> in the taper equation.  Array size is # behavior species.*/
  float *mp_fTaperI;

  /** <i>j</i> in the taper equation.  Array size is # behavior species.*/
  float *mp_fTaperJ;

  /** <i>k</i> in the taper equation.  Array size is # behavior species.*/
  float *mp_fTaperK;

  /** <i>a</i> in the math library's AddBarkToDBH() function.  Array size is
  * # behavior species.*/
  float *mp_fBarkA;

  /** <i>b</i> in the math library's AddBarkToDBH() function.  Array size is
  * # behavior species.*/
  float *mp_fBarkB;

  /** <i>c</i> in the math library's AddBarkToDBH() function.  Array size is
  * # behavior species.*/
  float *mp_fBarkC;

  /**For quick access to the other arrays.  Array size is # total species.*/
  short int *mp_iIndexes;

  /**String to pass to clTreePopulation::Find() in order to get the trees for
  * which to calculate volume.  This will instigate a species/type search for
  * all the species and types to which this behavior applies.*/
  char *m_cQuery;

  /**Stump height, in m.  Point at which to start summing trunk volume.  This
  * is read in as in cm, and then converted to m.*/
  float m_fStumpHeight;

  /**Minimum usable diameter, in cm.  Point at which to stop summing trunk
  * volume.*/
  float m_fMinUsableDiam;

  /**Length of tree trunk volume segments, in m*/
  float m_fSegmentLength;

  /**Holds data member codes for "Volume" float data member.  First array index
  * is # behavior species, second is number types (3 - sapling, adult, snag)*/
  short int **mp_iVolumeCodes;

  /**
  * Gets the volume of a tree trunk.
  * This function divides the trunk into segments and sums over the volumes
  * of each segment.  The segments begin at the height in mp_fStumpHeight and
  * are of length mp_fSegmentLength.  Summing stops upon reaching a segment
  * whose end diameter is less than mp_fMinUsableDiam, or until the end of the
  * tree is reached.
  * @param fTreeHeight Total tree height, in meters
  * @param fDBH SORTIE's DBH value, in cm
  * @param iSpecies Tree's species.
  * @return Tree volume in square meters.
  */
  float GetTreeVolume(const float &fTreeHeight, const float &fDBH,
                      const int &iSpecies);

  /**
  * Formats the string in m_cQuery.  This value will be used in Action() to
  * pass to clTreePopulation::Find() in order to get the trees to act on.
  */
  void FormatQueryString();
};
//---------------------------------------------------------------------------
#endif // VolumeCalculator_H

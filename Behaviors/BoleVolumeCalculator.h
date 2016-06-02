//---------------------------------------------------------------------------
// BoleVolumeCalculator
//---------------------------------------------------------------------------
#if !defined(BoleVolumeCalculator_H)
  #define BoleVolumeCalculator_H

#include "BehaviorBase.h"

/**
* Bole Volume Calculator Version 1.0
*
* This class calculates merchantable tree volume according to a US Forest
* Service equation. The volume value is stored in an float tree data member
* that this behavior adds as a value in cubic feet.
*
* Tree volume is calculated as follows:
*
* <center>V = b0 + b1*DBH<sup>b2</sup> + b3* DBH<sup>b4</sup> * Height<sup>b5</sup></center>
*
* Where:
* <ul>
* <li>V = gross cubic foot volume</li>
* <li>Height = bole length in feet, as a multiple of 16 (for usable 16-foot
* logs)</li>
* <li>DBH = dbh in inches</li>
* <li>b0 - b5 are parameters</li>
* </ul>
*
* The bole length is the number of 16-foot logs the tree can provide.  The base
* of the bole is the top of the cut stump (whatever that is; it's ignored by
* this behavior); the top of the bole is the merchantable height.  This
* behavior defines the merchantable height as the height at which the trunk
* diameter inside the bark tapers to 60% of DBH.
*
* To determine at what bole length the merchantable height occurs, we try
* fitting as many 16-foot logs as possible in to see how many fit before the
* 60% taper occurs.  The amount of taper at the top of the first 16-foot log is
* established by the form classes and is a fixed percentage entered by the user
* as a parameter.
*
* The amount of taper at the top of the first 16-foot log is subtracted from
* the DBH, to see how much taper is left before the 60% merchantable-height
* taper is reached.  There is no formula that establishes clearly how many logs
* will fit; we use a trial-and-error approach on a table until we find the
* correct number of logs.  The table below, adapted from Messavage and Girard,
* is contained in the code.  It represents the amount of taper at the top of
* the last log in a tree containing different numbers of logs.
* <table border=1>
* <tr><th>DBH (in)</th><th>2-log</th><th>3-log</th><th>4-log</th><th>5-log</th><th>6-log</th></tr>
* <tr><td>10</td><td>1.4</td><td>2.6</td><td>--</td><td>--</td><td>--</td></tr>
* <tr><td>12</td><td>1.6</td><td>2.8</td><td>4.4</td><td>--</td><td>--</td></tr>
* <tr><td>14</td><td>1.7</td><td>3</td><td>4.7</td><td>--</td><td>--</td></tr>
* <tr><td>16</td><td>1.9</td><td>3.2</td><td>4.9</td><td>--</td><td>--</td></tr>
* <tr><td>18</td><td>2</td><td>3.4</td><td>5.2</td><td>--</td><td>--</td></tr>
* <tr><td>20</td><td>2.1</td><td>3.6</td><td>5.6</td><td>7.8</td><td>--</td></tr>
* <tr><td>22</td><td>2.2</td><td>3.8</td><td>5.9</td><td>8</td><td>--</td></tr>
* <tr><td>24</td><td>2.3</td><td>4</td><td>6.3</td><td>8.4</td><td>--</td></tr>
* <tr><td>26</td><td>2.4</td><td>4.2</td><td>6.5</td><td>8.7</td><td>--</td></tr>
* <tr><td>28</td><td>2.5</td><td>4.4</td><td>6.8</td><td>9</td><td>12</td></tr>
* <tr><td>30</td><td>2.6</td><td>4.6</td><td>7.2</td><td>9.4</td><td>12.1</td></tr>
* <tr><td>32</td><td>2.7</td><td>4.7</td><td>7.3</td><td>9.9</td><td>12.3</td></tr>
* <tr><td>34</td><td>2.8</td><td>4.8</td><td>7.6</td><td>10.2</td><td>12.6</td></tr>
* <tr><td>36</td><td>2.8</td><td>4.9</td><td>7.8</td><td>10.4</td><td>13</td></tr>
* <tr><td>38</td><td>2.9</td><td>4.9</td><td>7.9</td><td>10.5</td><td>13.4</td></tr>
* <tr><td>40</td><td>2.9</td><td>5</td><td>8</td><td>10.9</td><td>13.9</td></tr>
</table>
*
* Consider a maple tree with a 20-inch DBH.  The user enters its form class as
* 79%.  We calculate the merchantable height to be that height at which the
* tree diameter inside the bark is 20 * 0.6 = 12 inches.  The diameter at the
* top of the first 16-foot log = 20 * 0.79 = 15.8 inches.
*
* The diameter at merchantable height is 12 inches; at the top of the first
* log it is 15.8 inches.  This means there is 3.8 inches of taper available
* left.  The next job is to use the lookup table to determine how many logs
* will fit into this amount of taper.
*
* Our DBH is 20 inches, which has an entry for itself in the lookup table.  If
* our DBH was not an even multiple of 2 inches, we would round down to the next
* lowest multiple of 2.  Any tree larger than 40 inches of DBH will use the
* 40-inch entry.  Trees must have a DBH of at least 10 inches to get a volume.
*
* We begin by assuming that our tree is a 2-log tree.  The amount of taper to
* the top of the second log in the table is given as 2.1.  This means our
* diameter at the top of the second log is 15.8 - 2.1 = 13.7 inches.  This is
* greater than 12 inches; so we will go back and try to fit three logs in.
*
* For a 3-log tree of 20 inches, the taper at the top of the third log is 3.6
* inches.  The diameter at the top of this log is 15.8 - 3.6 = 12.2.  This is
* still greater than 12, so we will try to fit in  four logs.
*
* The diameter at the top of the fourth log of a 4-log tree of 20 inches DBH
* is 15.8 - 1.4 - 1.8 - 2.4, or  10.2.  This is less than 12.  Thus, 4 logs
* don't fit.  Our bole length is 3 * 16 = 48 feet.
*
* This behavior adds an float data member called "Bole Vol" to trees that holds
* the final volume calculation as a value in square meters.
*
* This class's namestring and parameter call string are both
* "TreeBoleVolumeCalculator".
*
* This behavior may not be applied to seedlings.
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clBoleVolumeCalculator : virtual public clBehaviorBase {
  public:

  /**
  * Constructor.
  * @param p_oSimManager clSimManager object.
  */
  clBoleVolumeCalculator(clSimManager *p_oSimManager);

  /**
  * Destructor.  Deletes arrays.
  */
  ~clBoleVolumeCalculator();

  /**
  * Makes volume calculations.  A query is sent to the tree population to get
  * all trees to which this behavior is applied.  For each, the volume is
  * calculated by GetTreeVolume().  This value is placed in the "Bole Vol" float
  * tree data member.
  */
  void Action();

  /**
  * Does setup for this behavior.  Calls:
  * <ol>
  * <li>GetParameterFileData</li>
  * <li>FormatQueryString</li>
  * <li>PopulateTaperTable</li>
  * <li>Action() so that the initial conditions volume will be added</li>
  * </ol>
  * @param p_oDoc DOM tree of parsed input file.
  *
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
  * Registers the "Bole Vol" float data member.  The return codes are captured
  * in the mp_iVolumeCodes array.
  * @throw modelErr if this behavior is being applied to any tree type except
  * saplings, adults, and snags.
  */
  void RegisterTreeDataMembers();

  protected:
  /** <i>b0</i> in the volume equation.  Array size is # behavior species.*/
  double *mp_fB0;

  /** <i>b1</i> in the volume equation.  Array size is # behavior species.*/
  double *mp_fB1;

  /** <i>b2</i> in the volume equation.  Array size is # behavior species.*/
  double *mp_fB2;

  /** <i>b3</i> in the volume equation.  Array size is # behavior species.*/
  double *mp_fB3;

  /** <i>b4</i> in the volume equation.  Array size is # behavior species.*/
  double *mp_fB4;

  /** <i>b5</i> in the volume equation.  Array size is # behavior species.*/
  double *mp_fB5;

  /** Form classes.  This is the proportion of DBH that the diameter is at the
  * top of the first log.  This is entered by the user as a percentage between
  * 60 and 100 but to save math we convert to a proportion and subtract it from
  * 1 so it's the inverse.  Array size is # behavior species.*/
  double *mp_fFormClass;

  /** Taper table.  This is the amount by which diameter is reduced,
  * in inches, at the top of the last log in a 2-, 3-, 4-, 5-, or 6-log tree.
  * First array index is 16 (m_iDBHIncs, DBH in 2-in increments from 10 to 40
  * inches); second index is 6 (m_iMaxLogs).*/
  float **mp_fTaperTable;

  /**For quick access to the species-specific arrays.  Array size is # total
  * species.*/
  short int *mp_iIndexes;

  /**String to pass to clTreePopulation::Find() in order to get the trees for
  * which to calculate volume.  This will instigate a species/type search for
  * all the species and types to which this behavior applies.*/
  std::string m_sQuery;

  /**Holds data member codes for "Bole Vol" float data member.  First array index
  * is # behavior species, second is number types (3 - sapling, adult, snag)*/
  short int **mp_iVolumeCodes;

  /** The maximum number of logs a tree can have.  Equal to 6. */
  short int m_iMaxLogs;

  /** Number of DBH increments in the taper table.  Equal to 16 (DBH in 2-in
  * increments from 10 to 40 inches).*/
  short int m_iDBHIncs;

  /**
  * Gets the merchantable bole height of a tree.
  *
  * This function calculates the taper to the top of the first log using the
  * form class for the species.  After determining how much taper is left, it
  * uses the taper table to determine how many logs to add.  It determines the
  * DBH increment index by rounding down to the nearest even number and
  * subtracting 10 (with a max index of 15).  Then, for that index, it moves
  * from 0 to m_iMaxLogs across the table until it finds the greatest entry
  * that is less than or equal to the remaining taper.  That array index is
  * equal to the number of logs to add to the first one.  Then the result is
  * multiplied by 16 and returned.
  *
  * This function does not weed out trees with a DBH of less than 10 inches.
  *
  * @param fDBH SORTIE's DBH value, in in
  * @param iSpecies Tree's species.
  * @return Bole height as an integer number of feet.
  */
  int GetBoleHeight(const float &fDBH, const int &iSpecies);

  /**
  * Reads values from the parameter file.
  * @param p_oDoc DOM tree of parsed input file.
  * @throw modelErr if any value for form class is less than 60%.
  */
  void GetParameterFileData(xercesc::DOMDocument *p_oDoc);

  /**
  * Formats the string in m_sQuery.  This value will be used in Action() to
  * pass to clTreePopulation::Find() in order to get the trees to act on.
  */
  void FormatQueryString();

  /**
  * Populates the taper table.  The taper array is declared and the values
  * populated according to the values in the documentation above.
  */
  void PopulateTaperTable();
};
//---------------------------------------------------------------------------
#endif // VolumeCalculator_H

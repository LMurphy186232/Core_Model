//---------------------------------------------------------------------------
#ifndef ParsingFunctionsH
#define ParsingFunctionsH
//---------------------------------------------------------------------------
#include "DataTypes.h"

#include <xercesc/dom/DOM.hpp>

class clTreePopulation;

#include <fstream>

/**
 * @file
 * Data Extraction Functions
 * These functions are designed to work with the Xerces 2.1.0 library. They
 * handle the extraction of data from parsed files.
 *
 * Copyright 2003 Charles D. Canham.
 * @author Lora E. Murphy
 *
 * <br>Edit history:
 * <br>-----------------
 * <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
 * <br>November 12, 2012 - Chars became strings (LEM)
 */

/**
 * Fills species-specific float values from a DOM tree.
 * For those values which are species-specific and have the following structure
 * in the XML document:
 *
 * <!--<parent_tag>
 *    <child_tag species = "sp1-name">val1</child_tag>
 *    <child_tag species = "sp2-name">val2</child_tag
 * </parent_tag>-->
@htmlonly &lt;parent_tag&gt;<br>
     &lt;child_tag species = "sp1-name"&gt;val1&lt;/child_tag&gt;<br>
     &lt;child_tag species = "sp2-name"&gt;val2&lt;/child_tag&gt;<br>
&lt;/parent_tag&gt;<br>
@endhtmlonly
 *
 *
 * This function will extract the values and place them in a given array,
 * matching species codes. If the data isn't present in the document, the
 * action taken depends on the value of the flag bRequired. If bRequired is
 * false, and the data isn't there, the function simply exits. If bRequired is
 * true, the function throws an error. In either case, if a value is not found
 * for all species indicated, an error is thrown.
 *
 * The function will only look for values for species matching codes that are
 * pre-loaded into the arrays. The arrays must have had memory allocated and
 * the species codes must be pre-loaded. Duplicate and invalid species from the
 * XML file are screened out without failure.
 *
 * This function is currently not protected against overflow. I could not
 * successfully trap for the error codes the documentation says are supposed to
 * be set.
 *
 * @param p_oParent Pointer to the parent element of the element to extract. If
 * this is a top-level element, the parent will be the XML document root element
 * (at the time of this writing, named paramFile).
 * @param sTagName Tag name of the parent node (parent_tag).
 * @param sSubTagName Tag name of the child node (child_tag).
 * @param p_array Pointer to the array to put the extracted values in.
 * @param iNumSpecies Number of species expected (should equal the size of the
 * floatVal array)
 * @param p_oPop Pointer to the tree population. This is necessary for
 * translating species names into codes
 * @param bRequired Whether or not this value is required.
 * @throw Error if the value is required and it is not found, or if it is found
 * and not all species are present (whether or not it is required).
 */
void FillSpeciesSpecificValue(xercesc::DOMElement *p_oParent, std::string sTagName,
    std::string sSubTagName, floatVal *p_array, int iNumSpecies,
    clTreePopulation *p_oPop, bool bRequired);

/**
 * Fills species-specific double values from a DOM tree.
 * For those values which are species-specific and have the following structure
 * in the XML document:
 *
 * <!--<parent_tag>
 *    <child_tag species = "sp1-name">val1</child_tag>
 *    <child_tag species = "sp2-name">val2</child_tag
 * </parent_tag>-->
@htmlonly &lt;parent_tag&gt;<br>
     &lt;child_tag species = "sp1-name"&gt;val1&lt;/child_tag&gt;<br>
     &lt;child_tag species = "sp2-name"&gt;val2&lt;/child_tag&gt;<br>
&lt;/parent_tag&gt;<br>
@endhtmlonly
 *
 *
 * This function will extract the values and place them in a given array,
 * matching species codes. If the data isn't present in the document, the
 * action taken depends on the value of the flag bRequired. If bRequired is
 * false, and the data isn't there, the function simply exits. If bRequired is
 * true, the function throws an error. In either case, if a value is not found
 * for all species indicated, an error is thrown.
 *
 * The function will only look for values for species matching codes that are
 * pre-loaded into the arrays. The arrays must have had memory allocated and
 * the species codes must be pre-loaded. Duplicate and invalid species from the
 * XML file are screened out without failure.
 *
 * This function is currently not protected against overflow. I could not
 * successfully trap for the error codes the documentation says are supposed to
 * be set.
 *
 * @param p_oParent Pointer to the parent element of the element to extract. If
 * this is a top-level element, the parent will be the XML document root element
 * (at the time of this writing, named paramFile).
 * @param sTagName Tag name of the parent node (parent_tag).
 * @param sSubTagName Tag name of the child node (child_tag).
 * @param p_array Pointer to the array to put the extracted values in.
 * @param iNumSpecies Number of species expected (should equal the size of the
 * floatVal array)
 * @param p_oPop Pointer to the tree population. This is necessary for
 * translating species names into codes
 * @param bRequired Whether or not this value is required.
 * @throw Error if the value is required and it is not found, or if it is found
 * and not all species are present (whether or not it is required).
 */
void FillSpeciesSpecificValue(xercesc::DOMElement *p_oParent, std::string sTagName,
    std::string sSubTagName, doubleVal *p_array, int iNumSpecies,
    clTreePopulation *p_oPop, bool bRequired);

/**
 * Fills species-specific integer values from a DOM tree.
 * For those values which are species-specific and have the following structure
 * in the XML document:
 *
 * <!--<parent_tag>
 *   <child_tag species = "sp1-name">val1</child_tag>
 *   <child_tag species = "sp2-name">val2</child_tag>
 * </parent_tag>-->
@htmlonly &lt;parent_tag&gt;<br>
     &lt;child_tag species = "sp1-name"&gt;val1&lt;/child_tag&gt;<br>
     &lt;child_tag species = "sp2-name"&gt;val2&lt;/child_tag&gt;<br>
&lt;/parent_tag&gt;<br>
@endhtmlonly
 *
 * This function will extract the values and place them in a given array,
 * matching species codes. If the data isn't present in the document, the action
 * taken depends on the value of the flag bRequired. If bRequired is false, and
 * the data isn't there, the function simply exits. If bRequired is true, the
 * function throws an error. In either case, if a value is not found for all
 * species indicated, an error is thrown.
 *
 * The function will only look for values for species matching codes that
 * are pre-loaded into the arrays. The arrays must have had memory allocated and
 * the species codes must be pre-loaded. Duplicate and invalid species from the
 * XML file are screened out without failure.
 *
 * @param p_oParent Pointer to the parent element of the element to extract.
 * If this is a top-level element, the parent will be the XML document root
 * element (at the time of this writing, named paramFile).
 * @param sTagName Tag name of the parent node (parent_tag).
 * @param sSubTagName Tag name of the child node (child_tag).
 * @param p_array Pointer to the array to put the extracted values in.
 * @param iNumSpecies Number of species expected (should equal the size of the
 * intVal array)
 * @param p_oPop Pointer to the tree population. This is necessary for
 * translating species names into codes
 * @param bRequired Whether or not this value is required.
 * @throw Error if the value is required and it is not found, or if it is found
 * and not all species are present (whether or not it is required).
 */
void FillSpeciesSpecificValue(xercesc::DOMElement *p_oParent, std::string sTagName,
    std::string sSubTagName, intVal *p_array, int iNumSpecies,
    clTreePopulation *p_oPop, bool bRequired);


/**
 Fills species-specific boolean values from a DOM tree.
 For those values which are species-specific and have the following structure in
 the XML document:

<!--<parent_tag>
     <child_tag species = "sp1-name">val1</child_tag>
     <child_tag species = "sp2-name">val2</child_tag>
</parent_tag>-->
@htmlonly &lt;parent_tag&gt;<br>
     &lt;child_tag species = "sp1-name"&gt;val1&lt;/child_tag&gt;<br>
     &lt;child_tag species = "sp2-name"&gt;val2&lt;/child_tag&gt;<br>
&lt;/parent_tag&gt;<br>
@endhtmlonly
 <p>
 This function will extract the values and place them in a given array,
 matching species codes. If the data isn't present in the document, the action
 taken depends on the value of the flag bRequired. If bRequired is false, and
 the data isn't there, the function simply exits. If bRequired is true, the
 function throws an error. In either case, if a value is not found for all
 species indicated, an error is thrown.

 This expects the values to come as either 1 (true) or 0 (false).

 The function will only look for values for species matching codes that
 are pre-loaded into the arrays. The arrays must have had memory allocated and
 the species codes must be pre-loaded. Duplicate and invalid species from the
 XML file are screened out without failure.

 @param p_oParent Pointer to the parent element of the element to extract. If
 this is a top-level element, the parent will be the XML document root element
 (at the time of this writing, named paramFile).
 @param sTagName Tag name of the parent node (parent_tag).
 @param sSubTagName Tag name of the child node (child_tag).
 @param p_array Pointer to the array to put the extracted values in.
 @param iNumSpecies Number of species expected (should equal the size of the
 boolVal array)
 @param p_oPop Pointer to the tree population. This is necessary for
 translating species names into codes
 @param bRequired Whether or not this value is required.
 @throw Error if the value is required and it is not found, or if it is found
 and not all species are present (whether or not it is required).
 */
void FillSpeciesSpecificValue(xercesc::DOMElement *p_oParent, std::string sTagName,
    std::string sSubTagName, boolVal *p_array, int iNumSpecies,
    clTreePopulation *p_oPop, bool bRequired);


/**
 * Fills species-specific float values from a DOM tree.
 * For those values which are species-specific and have the following structure
 * in the XML document:
 *
 * <!--<parent_tag>
 *       <child_tag species = "sp1-name">val1</child_tag>
 *       <child_tag species = "sp2-name">val2</child_tag>
 * </parent_tag>-->
 @htmlonly &lt;parent_tag&gt;<br>
       &lt;child_tag species = "sp1-name"&gt;val1&lt;/child_tag&gt;<br>
       &lt;child_tag species = "sp2-name"&gt;val2&lt;/child_tag&gt;<br>
  &lt;/parent_tag&gt;<br>
 @endhtmlonly
 *
 * This function will extract the values and place them in a given array. If the
 * data isn't present in the document, the action taken depends on the value of
 * the flag bRequired. If bRequired is false, and the data isn't there, the
 * function simply exits. If bRequired is true, the function throws an error.
 * In either case, if a value is not found for all species indicated, an error
 * is thrown.
 *
 * This function provides a shortcut if you need values for every species and
 * don't need to bother with the floatVal array type.
 *
 * This function is currently not protected against overflow. I could not
 * successfully trap for the error codes the documentation says are supposed to
 * be set.
 *
 * @param p_oParent Pointer to the parent element of the element to extract. If
 * this is a top-level element, the parent will be the XML document root element
 * (at the time of this writing, named paramFile).
 * @param sTagName Tag name of the parent node (parent_tag).
 * @param sSubTagName Tag name of the child node (child_tag).
 * @param p_fArray Pointer to the array to put the extracted values in.
 * @param p_oPop Pointer to the tree population. This is necessary for
 * translating species names into codes
 * @param bRequired Whether or not this value is required.
 * @throw Error if the value is required and it is not found, or if it is found
 * and not all species are present (whether or not it is required).
 */
void FillSpeciesSpecificValue(xercesc::DOMElement *p_oParent, std::string sTagName,
    std::string sSubTagName, float *p_fArray, clTreePopulation *p_oPop,
    bool bRequired);

/**
 * Fills species-specific integer values from a DOM tree.
 * For those values which are species-specific and have the following structure
 * in the XML document:
 *
 * <!--<parent_tag>
 *       <child_tag species = "sp1-name">val1</child_tag>
 *       <child_tag species = "sp2-name">val2</child_tag>
 * </parent_tag>-->
 @htmlonly &lt;parent_tag&gt;<br>
       &lt;child_tag species = "sp1-name"&gt;val1&lt;/child_tag&gt;<br>
       &lt;child_tag species = "sp2-name"&gt;val2&lt;/child_tag&gt;<br>
 &lt;/parent_tag&gt;<br>
 @endhtmlonly
 *
 * This function will extract the values and place them in a given array. If the
 * data isn't present in the document, the action taken depends on the value of
 * the flag bRequired. If bRequired is false, and the data isn't there, the
 * function simply exits. If bRequired is true, the function throws an error.
 * In either case, if a value is not found for all species indicated, an error
 * is thrown.
 *
 * This function provides a shortcut if you need values for every species and
 * don't need to bother with the intVal array type.
 *
 * This function is currently not protected against overflow. I could not
 * successfully trap for the error codes the documentation says are supposed to
 * be set.
 *
 * @param p_oParent Pointer to the parent element of the element to extract. If
 * this is a top-level element, the parent will be the XML document root element
 * (at the time of this writing, named paramFile).
 * @param sTagName Tag name of the parent node (parent_tag).
 * @param sSubTagName Tag name of the child node (child_tag).
 * @param p_iArray Pointer to the array to put the extracted values in.
 * @param p_oPop Pointer to the tree population. This is necessary for
 * translating species names into codes
 * @param bRequired Whether or not this value is required.
 * @throw Error if the value is required and it is not found, or if it is found
 * and not all species are present (whether or not it is required).
 */
void FillSpeciesSpecificValue(xercesc::DOMElement *p_oParent, std::string sTagName,
    std::string sSubTagName, int *p_iArray, clTreePopulation *p_oPop,
    bool bRequired);

/**
 * Translates a species string into a species number. For those nodes that have
 * an attribute named "species", this will extract the name of the species in
 * that attribute and turn it into a code.
 *
 * @param p_oDocNode Node that has the species attribute to translate.
 * @param p_oPop Tree population object.
 * @return Species code. If the species name was unrecognized, returns -1.
 */
short int GetNodeSpeciesCode(xercesc::DOMNode *p_oDocNode, clTreePopulation *p_oPop);

/**
 * Extracts a single integer value from a parsed XML file.
 * For those values which are not species-specific and have the following
 * structure:
 *
 @htmlonly &lt;tag&gt;value&lt;/tag&gt; @endhtmlonly
 * <!-- <tag>value</tag>-->
 *
 * this function will extract the value and place it in a given variable. The
 * element can be the child of another tag.
 *
 * @param p_oParent Pointer to the parent element of the element to extract. If
 * this is a top-level element, the parent will be the XML document root
 * element (at the time of this writing, named paramFile).
 * @param sTagName The tag name of the node.
 * @param p_iValToFill Pointer to the variable to put the extracted value in.
 * @param bRequired Whether or not this value is required.
 * @throw Error if the value is required and it is not found.
 */
void FillSingleValue(xercesc::DOMElement *p_oParent, std::string sTagName,
    int *p_iValToFill, bool bRequired);

/**
 * Extracts a single float value from a parsed XML file.
 * For those values which are not species-specific and have the following
 * structure:
 *
 @htmlonly &lt;tag&gt;value&lt;/tag&gt; @endhtmlonly
 * <!-- <tag>value</tag>-->
 *
 * this function will extract the value and place it in a given variable. The
 * element can be the child of another tag.
 *
 * This is currently not protected against overflow. I could not successfully
 * trap for the error codes the documentation says are supposed to be set.
 *
 * @param p_oParent Pointer to the parent element of the element to extract. If
 * this is a top-level element, the parent will be the XML document root
 * element (at the time of this writing, named paramFile).
 * @param sTagName The tag name of the node.
 * @param p_fValToFill Pointer to the variable to put the extracted value in.
 * @param bRequired Whether or not this value is required.
 * @throw Error if the value is required and it is not found.
 */
void FillSingleValue(xercesc::DOMElement *p_oParent, std::string sTagName,
    float *p_fValToFill, bool bRequired);

/**
 * Extracts a single string value from a parsed XML file.
 * For those values which are not species-specific and have the following
 * structure:
 *
 *     @htmlonly &lt;tag&gt;value&lt;/tag&gt; @endhtmlonly
 *     <!-- <tag>value</tag>-->
 *
 * this function will extract the value and place it in a given variable. The
 * element can be the child of another tag. An empty string is considered a
 * valid value.
 *
 * @param p_oParent Pointer to the parent element of the element to extract.
 * If this is a top-level element, the parent will be the XML document root
 * element (at the time of this writing, named paramFile).
 * @param sTagName The tag name of the node.
 * @param p_sValToFill Pointer to the variable to put the extracted value in.
 * @param bRequired Whether or not this value is required.
 * @throw Error if the value is required and it is not found.
 */
void FillSingleValue(xercesc::DOMElement *p_oParent, std::string sTagName,
    std::string *p_sValToFill, bool bRequired);

/**
 * Extracts a single boolean value from a parsed XML file.
 * For those values which are not species-specific and have the following
 * structure:
 *
 @htmlonly &lt;tag&gt;value&lt;/tag&gt; @endhtmlonly
 * <!-- <tag>value</tag>-->
 *
 * this function will extract the value and place it in a given variable. The
 * element can be the child of another tag.
 *
 * This expects the values to come as either 1 (true) or 0 (false).
 *
 * @param p_oParent Pointer to the parent element of the element to extract. If
 * this is a top-level element, the parent will be the XML document root
 * element (at the time of this writing, named paramFile).
 * @param sTagName The tag name of the node.
 * @param p_bValToFill Pointer to the variable to put the extracted value in.
 * @param bRequired Whether or not this value is required.
 * @throw Error if the value is required and it is not found.
 */
void FillSingleValue(xercesc::DOMElement *p_oParent, std::string sTagName,
    bool *p_bValToFill, bool bRequired);
//---------------------------------------------------------------------------
#endif

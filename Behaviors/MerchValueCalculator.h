//---------------------------------------------------------------------------
// MerchValueCalculator
//---------------------------------------------------------------------------
#if !defined(MerchValueCalculator_H)
  #define MerchValueCalculator_H

#include "BehaviorBase.h"

class clGrid;

/**
* Merchantable Timber Value Calculator Version 1.0
*
* This class calculates merchantable tree timber value according to a US Forest
* Service equation. The value is stored in an float tree data member that
* this behavior adds, as a value in an unspecified currency (user's choice).
* This also creates a grid object with species total values.
*
* The value is price per thousand board feet of timber.  The amount of timber
* in a tree is based on the size of the tree and the number of 16-foot logs
* that can be extracted from a tree.
*
* The number of 16-foot logs a tree can provide is based on the bole length.
* The base of the bole is the top of the cut stump (whatever that is; it's
* ignored by this behavior); the top of the bole is the merchantable height.
* This behavior defines the merchantable height as the height at which the
* trunk diameter inside the bark tapers to 60% of DBH.
*
* There's no simple equation to determine how many 16 foot logs fit into the
* length of the bole. This behavior uses a table along with trial-and-error to
* fit as many 16-foot logs as possible in before the 60% taper occurs.  The
* amount of taper at the top of the first 16-foot log is established by the
* form classes and is a fixed percentage entered by the user as a parameter.
*
* All trees above the minimum DBH are assumed to have at least 1 16-foot log.
* The amount of taper at the top of this first log is subtracted from the DBH,
* to see how much taper is left before the 60% merchantable-height taper is
* reached. The table below, adapted from Messavage and Girard, is contained
* in the code. It represents the amount of taper at the top of the last log
* in a tree containing different numbers of logs.
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
* left. The next job is to use the lookup table to determine how many logs
* will fit into this amount of taper.
*
* Our DBH is 20 inches, which has an entry for itself in the lookup table.  If
* our DBH was not an even multiple of 2 inches, we would round down to the next
* lowest multiple of 2. Any tree larger than 40 inches of DBH will use the
* 40-inch entry. Trees must have a DBH of at least 10 inches to get a volume.
*
* We begin by assuming that our tree is a 2-log tree. The amount of taper to
* the top of the second log in the table is given as 2.1. This means our
* diameter at the top of the second log is 15.8 - 2.1 = 13.7 inches. This is
* greater than 12 inches; so we will go back and try to fit three logs in.
*
* For a 3-log tree of 20 inches, the taper at the top of the third log is 3.6
* inches. The diameter at the top of this log is 15.8 - 3.6 = 12.2. This is
* still greater than 12, so we will try to fit in  four logs.
*
* The diameter at the top of the fourth log of a 4-log tree of 20 inches DBH
* is 15.8 - 1.4 - 1.8 - 2.4, or  10.2.  This is less than 12.  Thus, 4 logs
* don't fit.  Our tree thus provides 3 16-foot logs.
*
* Once we know the number of 16-foot logs, then we can use another set of
* tables to find out how many board-feet of timber those logs provide.  The
* table used is based on form class (again, from Messavage and Girard).  The
* timber is assumed to be 0.25" in width.
*
* <b>VOLUME (BOARD FEET) BY NUMBER OF USABLE 16 FOOT LOGS</b><br>
* <b>Form Class 78:</b>
* <table border=1>
* <tr><th>DBH (inches)</th><th>1 log</th><th>2 logs</th><th>3 logs</th><th>4 logs</th><th>5 logs</th><th>6 logs</th></tr>
* <tr><td>10</td><td>36</td><td>59</td><td>73</td><td>--</td><td>--</td><td>--</td></tr>
* <tr><td>11</td><td>46</td><td>76</td><td>96</td><td>--</td><td>--</td><td>--</td></tr>
* <tr><td>12</td><td>56</td><td>92</td><td>120</td><td>137</td><td>--</td><td>--</td></tr>
* <tr><td>13</td><td>67</td><td>112</td><td>147</td><td>168</td><td>--</td><td>--</td></tr>
* <tr><td>14</td><td>78</td><td>132</td><td>174</td><td>200</td><td>--</td><td>--</td></tr>
* <tr><td>15</td><td>92</td><td>156</td><td>208</td><td>242</td><td>--</td><td>--</td></tr>
* <tr><td>16</td><td>106</td><td>180</td><td>241</td><td>285</td><td>--</td><td>--</td></tr>
* <tr><td>17</td><td>121</td><td>206</td><td>278</td><td>330</td><td>--</td><td>--</td></tr>
* <tr><td>18</td><td>136</td><td>233</td><td>314</td><td>374</td><td>--</td><td>--</td></tr>
* <tr><td>19</td><td>154</td><td>264</td><td>358</td><td>427</td><td>--</td><td>--</td></tr>
* <tr><td>20</td><td>171</td><td>296</td><td>401</td><td>480</td><td>542</td><td>--</td></tr>
* <tr><td>21</td><td>191</td><td>332</td><td>450</td><td>542</td><td>616</td><td>--</td></tr>
* <tr><td>22</td><td>211</td><td>368</td><td>500</td><td>603</td><td>691</td><td>--</td></tr>
* <tr><td>23</td><td>231</td><td>404</td><td>552</td><td>663</td><td>714</td><td>--</td></tr>
* <tr><td>24</td><td>251</td><td>441</td><td>605</td><td>723</td><td>782</td><td>--</td></tr>
* <tr><td>25</td><td>275</td><td>484</td><td>665</td><td>800</td><td>865</td><td>--</td></tr>
* <tr><td>26</td><td>299</td><td>528</td><td>725</td><td>877</td><td>1,021</td><td>--</td></tr>
* <tr><td>27</td><td>323</td><td>572</td><td>788</td><td>952</td><td>1,111</td><td>--</td></tr>
* <tr><td>28</td><td>347</td><td>616</td><td>850</td><td>1,027</td><td>1,201</td><td>1,358</td></tr>
* <tr><td>29</td><td>375</td><td>667</td><td>920</td><td>1,112</td><td>1,308</td><td>1,488</td></tr>
* <tr><td>30</td><td>403</td><td>718</td><td>991</td><td>1,198</td><td>1,415</td><td>1,619</td></tr>
* <tr><td>31</td><td>432</td><td>772</td><td>1,070</td><td>1,299</td><td>1,526</td><td>1,754</td></tr>
* <tr><td>32</td><td>462</td><td>826</td><td>1,149</td><td>1,400</td><td>1,637</td><td>1,888</td></tr>
* <tr><td>33</td><td>492</td><td>880</td><td>1,226</td><td>1,495</td><td>1,750</td><td>2,026</td></tr>
* <tr><td>34</td><td>521</td><td>934</td><td>1,304</td><td>1,590</td><td>1,864</td><td>2,163</td></tr>
* <tr><td>35</td><td>555</td><td>998</td><td>1,394</td><td>1,702</td><td>2,000</td><td>2,312</td></tr>
* <tr><td>36</td><td>589</td><td>1,063</td><td>1,485</td><td>1,814</td><td>2,135</td><td>2,461</td></tr>
* <tr><td>37</td><td>622</td><td>1,124</td><td>1,578</td><td>1,926</td><td>2,272</td><td>2,616</td></tr>
* <tr><td>38</td><td>656</td><td>1,186</td><td>1,670</td><td>2,038</td><td>2,410</td><td>2,771</td></tr>
* <tr><td>39</td><td>694</td><td>1,258</td><td>1,769</td><td>2,166</td><td>2,552</td><td>2,937</td></tr>
* <tr><td>40</td><td>731</td><td>1,329</td><td>1,868</td><td>2,294</td><td>2,693</td><td>3,103</td></tr>
* </table>
*
* <br><b>Form Class 79:</b>
* <table border=1>
* <tr><th>DBH (inches)</th><th>1 log</th><th>2 logs</th><th>3 logs</th><th>4 logs</th><th>5 logs</th><th>6 logs</th></tr>
* <tr><td>10</td><td>38</td><td>61</td><td>77</td><td>--</td><td>--</td><td>--</td></tr>
* <tr><td>11</td><td>48</td><td>78</td><td>100</td><td>--</td><td>--</td><td>--</td></tr>
* <tr><td>12</td><td>58</td><td>96</td><td>124</td><td>141</td><td>--</td><td>--</td></tr>
* <tr><td>13</td><td>70</td><td>117</td><td>153</td><td>176</td><td>--</td><td>--</td></tr>
* <tr><td>14</td><td>82</td><td>138</td><td>182</td><td>211</td><td>--</td><td>--</td></tr>
* <tr><td>15</td><td>95</td><td>160</td><td>214</td><td>252</td><td>--</td><td>--</td></tr>
* <tr><td>16</td><td>108</td><td>183</td><td>246</td><td>292</td><td>--</td><td>--</td></tr>
* <tr><td>17</td><td>124</td><td>212</td><td>286</td><td>340</td><td>--</td><td>--</td></tr>
* <tr><td>18</td><td>140</td><td>240</td><td>325</td><td>388</td><td>--</td><td>--</td></tr>
* <tr><td>19</td><td>158</td><td>272</td><td>370</td><td>442</td><td>--</td><td>--</td></tr>
* <tr><td>20</td><td>176</td><td>305</td><td>414</td><td>496</td><td>561</td><td>--</td></tr>
* <tr><td>21</td><td>196</td><td>342</td><td>464</td><td>558</td><td>636</td><td>--</td></tr>
* <tr><td>22</td><td>216</td><td>378</td><td>514</td><td>621</td><td>710</td><td>--</td></tr>
* <tr><td>23</td><td>238</td><td>418</td><td>571</td><td>687</td><td>792</td><td>--</td></tr>
* <tr><td>24</td><td>260</td><td>458</td><td>628</td><td>753</td><td>875</td><td>--</td></tr>
* <tr><td>25</td><td>282</td><td>499</td><td>685</td><td>826</td><td>960</td><td>--</td></tr>
* <tr><td>26</td><td>305</td><td>540</td><td>742</td><td>899</td><td>1,046</td><td>--</td></tr>
* <tr><td>27</td><td>331</td><td>588</td><td>810</td><td>980</td><td>1,144</td><td>--</td></tr>
* <tr><td>28</td><td>357</td><td>635</td><td>877</td><td>1,061</td><td>1,242</td><td>1,404</td></tr>
* <tr><td>29</td><td>385</td><td>686</td><td>948</td><td>1,148</td><td>1,350</td><td>1,537</td></tr>
* <tr><td>30</td><td>413</td><td>737</td><td>1,020</td><td>1,235</td><td>1,458</td><td>1,670</td></tr>
* <tr><td>31</td><td>444</td><td>792</td><td>1,100</td><td>1,338</td><td>1,572</td><td>1,808</td></tr>
* <tr><td>32</td><td>474</td><td>848</td><td>1,181</td><td>1,440</td><td>1,685</td><td>1,945</td></tr>
* <tr><td>33</td><td>506</td><td>907</td><td>1,265</td><td>1,544</td><td>1,808</td><td>2,094</td></tr>
* <tr><td>34</td><td>538</td><td>966</td><td>1,349</td><td>1,647</td><td>1,932</td><td>2,244</td></tr>
* <tr><td>35</td><td>570</td><td>1,026</td><td>1,435</td><td>1,754</td><td>2,000</td><td>2,384</td></tr>
* <tr><td>36</td><td>602</td><td>1,087</td><td>1,521</td><td>1,860</td><td>2,189</td><td>2,525</td></tr>
* <tr><td>37</td><td>638</td><td>1,154</td><td>1,620</td><td>1,980</td><td>2,338</td><td>2,694</td></tr>
* <tr><td>38</td><td>674</td><td>1,220</td><td>1,720</td><td>2,101</td><td>2,488</td><td>2,862</td></tr>
* <tr><td>39</td><td>712</td><td>1,292</td><td>1,822</td><td>2,232</td><td>2,632</td><td>3,031</td></tr>
* <tr><td>40</td><td>750</td><td>1,365</td><td>1,923</td><td>2,362</td><td>2,775</td><td>3,200</td></tr>
* </table>
*
* <br><b>Form Class 80:</b>
* <table border=1>
* <tr><th>DBH (inches)</th><th>1 log</th><th>2 logs</th><th>3 logs</th><th>4 logs</th><th>5 logs</th><th>6 logs</th></tr>
* <tr><td>10</td><td>39</td><td>63</td><td>80</td><td>--</td><td>--</td><td>--</td></tr>
* <tr><td>11</td><td>49</td><td>80</td><td>104</td><td>--</td><td>--</td><td>--</td></tr>
* <tr><td>12</td><td>59</td><td>98</td><td>127</td><td>146</td><td>--</td><td>--</td></tr>
* <tr><td>13</td><td>71</td><td>120</td><td>156</td><td>181</td><td>--</td><td>--</td></tr>
* <tr><td>14</td><td>83</td><td>141</td><td>186</td><td>216</td><td>--</td><td>--</td></tr>
* <tr><td>15</td><td>98</td><td>166</td><td>221</td><td>260</td><td>--</td><td>--</td></tr>
* <tr><td>16</td><td>112</td><td>190</td><td>256</td><td>305</td><td>--</td><td>--</td></tr>
* <tr><td>17</td><td>128</td><td>219</td><td>296</td><td>354</td><td>--</td><td>--</td></tr>
* <tr><td>18</td><td>144</td><td>248</td><td>336</td><td>402</td><td>--</td><td>--</td></tr>
* <tr><td>19</td><td>162</td><td>281</td><td>382</td><td>457</td><td>--</td><td>--</td></tr>
* <tr><td>20</td><td>181</td><td>314</td><td>427</td><td>512</td><td>580</td><td>--</td></tr>
* <tr><td>21</td><td>201</td><td>350</td><td>478</td><td>575</td><td>656</td><td>--</td></tr>
* <tr><td>22</td><td>221</td><td>387</td><td>528</td><td>638</td><td>732</td><td>--</td></tr>
* <tr><td>23</td><td>244</td><td>428</td><td>586</td><td>706</td><td>816</td><td>--</td></tr>
* <tr><td>24</td><td>266</td><td>469</td><td>644</td><td>773</td><td>899</td><td>--</td></tr>
* <tr><td>25</td><td>290</td><td>514</td><td>706</td><td>852</td><td>992</td><td>--</td></tr>
* <tr><td>26</td><td>315</td><td>558</td><td>767</td><td>931</td><td>1,086</td><td>--</td></tr>
* <tr><td>27</td><td>341</td><td>606</td><td>836</td><td>1,014</td><td>1,185</td><td>--</td></tr>
* <tr><td>28</td><td>367</td><td>654</td><td>904</td><td>1,096</td><td>1,284</td><td>1,453</td></tr>
* <tr><td>29</td><td>396</td><td>706</td><td>977</td><td>1,184</td><td>1,394</td><td>1,588</td></tr>
* <tr><td>30</td><td>424</td><td>758</td><td>1,050</td><td>1,272</td><td>1,503</td><td>1,723</td></tr>
* <tr><td>31</td><td>454</td><td>814</td><td>1,132</td><td>1,376</td><td>1,618</td><td>1,862</td></tr>
* <tr><td>32</td><td>485</td><td>870</td><td>1,213</td><td>1,480</td><td>1,733</td><td>2,001</td></tr>
* <tr><td>33</td><td>518</td><td>930</td><td>1,298</td><td>1,586</td><td>1,858</td><td>2,152</td></tr>
* <tr><td>34</td><td>550</td><td>989</td><td>1,383</td><td>1,691</td><td>1,984</td><td>2,304</td></tr>
* <tr><td>35</td><td>585</td><td>1,055</td><td>1,477</td><td>1,806</td><td>2,124</td><td>2,458</td></tr>
* <tr><td>36</td><td>620</td><td>1,121</td><td>1,571</td><td>1,922</td><td>2,264</td><td>2,612</td></tr>
* <tr><td>37</td><td>656</td><td>1,188</td><td>1,672</td><td>2,044</td><td>2,416</td><td>2,783</td></tr>
* <tr><td>38</td><td>693</td><td>1,256</td><td>1,772</td><td>2,167</td><td>2,568</td><td>2,954</td></tr>
* <tr><td>39</td><td>732</td><td>1,330</td><td>1,874</td><td>2,300</td><td>2,714</td><td>3,127</td></tr>
* <tr><td>40</td><td>770</td><td>1,403</td><td>1,977</td><td>2,432</td><td>2,860</td><td>3,300</td></tr>
* </table>
*
* <br><b>Form Class 81:</b>
* <table border=1>
* <tr><th>DBH (inches)</th><th>1 log</th><th>2 logs</th><th>3 logs</th><th>4 logs</th><th>5 logs</th><th>6 logs</th></tr>
* <tr><td>10</td><td>40</td><td>65</td><td>82</td><td>--</td><td>--</td><td>--</td></tr>
* <tr><td>11</td><td>50</td><td>82</td><td>106</td><td>--</td><td>--</td><td>--</td></tr>
* <tr><td>12</td><td>60</td><td>100</td><td>130</td><td>150</td><td>--</td><td>--</td></tr>
* <tr><td>13</td><td>72</td><td>122</td><td>160</td><td>186</td><td>--</td><td>--</td></tr>
* <tr><td>14</td><td>85</td><td>144</td><td>190</td><td>221</td><td>--</td><td>--</td></tr>
* <tr><td>15</td><td>100</td><td>170</td><td>228</td><td>268</td><td>--</td><td>--</td></tr>
* <tr><td>16</td><td>115</td><td>197</td><td>265</td><td>316</td><td>--</td><td>--</td></tr>
* <tr><td>17</td><td>132</td><td>226</td><td>306</td><td>366</td><td>--</td><td>--</td></tr>
* <tr><td>18</td><td>148</td><td>256</td><td>346</td><td>415</td><td>--</td><td>--</td></tr>
* <tr><td>19</td><td>166</td><td>290</td><td>392</td><td>471</td><td>--</td><td>--</td></tr>
* <tr><td>20</td><td>185</td><td>323</td><td>439</td><td>527</td><td>598</td><td>--</td></tr>
* <tr><td>21</td><td>206</td><td>360</td><td>492</td><td>592</td><td>676</td><td>--</td></tr>
* <tr><td>22</td><td>227</td><td>398</td><td>544</td><td>656</td><td>754</td><td>--</td></tr>
* <tr><td>23</td><td>250</td><td>439</td><td>602</td><td>724</td><td>838</td><td>--</td></tr>
* <tr><td>24</td><td>272</td><td>480</td><td>659</td><td>791</td><td>923</td><td>--</td></tr>
* <tr><td>25</td><td>298</td><td>528</td><td>726</td><td>877</td><td>1,024</td><td>--</td></tr>
* <tr><td>26</td><td>324</td><td>575</td><td>793</td><td>963</td><td>1,124</td><td>--</td></tr>
* <tr><td>27</td><td>351</td><td>624</td><td>863</td><td>1,047</td><td>1,226</td><td>--</td></tr>
* <tr><td>28</td><td>378</td><td>674</td><td>933</td><td>1,131</td><td>1,327</td><td>1,502</td></tr>
* <tr><td>29</td><td>406</td><td>726</td><td>1,006</td><td>1,220</td><td>1,438</td><td>1,640</td></tr>
* <tr><td>30</td><td>435</td><td>779</td><td>1,080</td><td>1,310</td><td>1,549</td><td>1,777</td></tr>
* <tr><td>31</td><td>466</td><td>836</td><td>1,162</td><td>1,416</td><td>1,666</td><td>1,918</td></tr>
* <tr><td>32</td><td>497</td><td>892</td><td>1,245</td><td>1,522</td><td>1,784</td><td>2,059</td></tr>
* <tr><td>33</td><td>530</td><td>953</td><td>1,332</td><td>1,628</td><td>1,910</td><td>2,214</td></tr>
* <tr><td>34</td><td>563</td><td>1,014</td><td>1,419</td><td>1,734</td><td>2,037</td><td>2,368</td></tr>
* <tr><td>35</td><td>600</td><td>1,084</td><td>1,518</td><td>1,859</td><td>2,188</td><td>2,534</td></tr>
* <tr><td>36</td><td>637</td><td>1,154</td><td>1,618</td><td>1,984</td><td>2,338</td><td>2,700</td></tr>
* <tr><td>37</td><td>674</td><td>1,223</td><td>1,721</td><td>2,109</td><td>2,494</td><td>2,874</td></tr>
* <tr><td>38</td><td>712</td><td>1,292</td><td>1,824</td><td>2,234</td><td>2,649</td><td>3,049</td></tr>
* <tr><td>39</td><td>751</td><td>1,366</td><td>1,928</td><td>2,368</td><td>2,796</td><td>3,224</td></tr>
* <tr><td>40</td><td>790</td><td>1,441</td><td>2,032</td><td>2,502</td><td>2,944</td><td>3,399</td></tr>
* </table>
*
* <br><b>Form Class 84:</b>
* <table border=1>
* <tr><th>DBH (inches)</th><th>1 log</th><th>2 logs</th><th>3 logs</th><th>4 logs</th><th>5 logs</th><th>6 logs</th></tr>
* <tr><td>10</td><td>43</td><td>71</td><td>91</td><td>--</td><td>--</td><td>--</td></tr>
* <tr><td>11</td><td>54</td><td>91</td><td>118</td><td>--</td><td>--</td><td>--</td></tr>
* <tr><td>12</td><td>66</td><td>111</td><td>145</td><td>168</td><td>--</td><td>--</td></tr>
* <tr><td>13</td><td>80</td><td>135</td><td>178</td><td>208</td><td>--</td><td>--</td></tr>
* <tr><td>14</td><td>93</td><td>159</td><td>212</td><td>248</td><td>--</td><td>--</td></tr>
* <tr><td>15</td><td>108</td><td>185</td><td>249</td><td>295</td><td>--</td><td>--</td></tr>
* <tr><td>16</td><td>123</td><td>211</td><td>286</td><td>342</td><td>--</td><td>--</td></tr>
* <tr><td>17</td><td>142</td><td>244</td><td>332</td><td>398</td><td>--</td><td>--</td></tr>
* <tr><td>18</td><td>160</td><td>277</td><td>377</td><td>453</td><td>--</td><td>--</td></tr>
* <tr><td>19</td><td>180</td><td>314</td><td>428</td><td>524</td><td>--</td><td>--</td></tr>
* <tr><td>20</td><td>200</td><td>351</td><td>479</td><td>576</td><td>657</td><td>--</td></tr>
* <tr><td>21</td><td>223</td><td>392</td><td>537</td><td>649</td><td>744</td><td>--</td></tr>
* <tr><td>22</td><td>246</td><td>434</td><td>595</td><td>722</td><td>830</td><td>--</td></tr>
* <tr><td>23</td><td>271</td><td>480</td><td>660</td><td>798</td><td>925</td><td>--</td></tr>
* <tr><td>24</td><td>296</td><td>525</td><td>724</td><td>873</td><td>1,020</td><td>--</td></tr>
* <tr><td>25</td><td>322</td><td>572</td><td>790</td><td>958</td><td>1,118</td><td>--</td></tr>
* <tr><td>26</td><td>347</td><td>619</td><td>855</td><td>1,042</td><td>1,217</td><td>--</td></tr>
* <tr><td>27</td><td>376</td><td>673</td><td>932</td><td>1,136</td><td>1,331</td><td>--</td></tr>
* <tr><td>28</td><td>406</td><td>727</td><td>1,010</td><td>1,230</td><td>1,445</td><td>1,636</td></tr>
* <tr><td>29</td><td>438</td><td>786</td><td>1,092</td><td>1,330</td><td>1,569</td><td>1,790</td></tr>
* <tr><td>30</td><td>470</td><td>844</td><td>1,173</td><td>1,429</td><td>1,693</td><td>1,943</td></tr>
* <tr><td>31</td><td>504</td><td>907</td><td>1,265</td><td>1,546</td><td>1,823</td><td>2,101</td></tr>
* <tr><td>32</td><td>538</td><td>970</td><td>1,357</td><td>1,664</td><td>1,953</td><td>2,259</td></tr>
* <tr><td>33</td><td>574</td><td>1,037</td><td>1,453</td><td>1,782</td><td>2,096</td><td>2,431</td></tr>
* <tr><td>34</td><td>611</td><td>1,104</td><td>1,549</td><td>1,901</td><td>2,240</td><td>2,603</td></tr>
* <tr><td>35</td><td>647</td><td>1,173</td><td>1,648</td><td>2,023</td><td>2,387</td><td>2,766</td></tr>
* <tr><td>36</td><td>683</td><td>1,242</td><td>1,746</td><td>2,145</td><td>2,534</td><td>2,929</td></tr>
* <tr><td>37</td><td>724</td><td>1,318</td><td>1,859</td><td>2,284</td><td>2,706</td><td>3,123</td></tr>
* <tr><td>38</td><td>765</td><td>1,393</td><td>1,972</td><td>2,422</td><td>2,877</td><td>3,317</td></tr>
* <tr><td>39</td><td>808</td><td>1,476</td><td>2,088</td><td>2,570</td><td>3,042</td><td>3,512</td></tr>
* <tr><td>40</td><td>851</td><td>1,558</td><td>2,203</td><td>2,719</td><td>3,208</td><td>3,706</td></tr>
* </table>
*
* <br><b>Form Class 85:</b>
* <table border=1>
* <tr><th>DBH (inches)</th><th>1 log</th><th>2 logs</th><th>3 logs</th><th>4 logs</th><th>5 logs</th><th>6 logs</th></tr>
* <tr><td>10</td><td>45</td><td>74</td><td>94</td><td>--</td><td>--</td><td>--</td></tr>
* <tr><td>11</td><td>56</td><td>94</td><td>122</td><td>--</td><td>--</td><td>--</td></tr>
* <tr><td>12</td><td>68</td><td>114</td><td>150</td><td>173</td><td>--</td><td>--</td></tr>
* <tr><td>13</td><td>82</td><td>138</td><td>184</td><td>214</td><td>--</td><td>--</td></tr>
* <tr><td>14</td><td>95</td><td>163</td><td>217</td><td>254</td><td>--</td><td>--</td></tr>
* <tr><td>15</td><td>111</td><td>191</td><td>257</td><td>304</td><td>--</td><td>--</td></tr>
* <tr><td>16</td><td>127</td><td>219</td><td>297</td><td>355</td><td>--</td><td>--</td></tr>
* <tr><td>17</td><td>146</td><td>252</td><td>342</td><td>412</td><td>--</td><td>--</td></tr>
* <tr><td>18</td><td>164</td><td>285</td><td>388</td><td>468</td><td>--</td><td>--</td></tr>
* <tr><td>19</td><td>184</td><td>322</td><td>440</td><td>531</td><td>--</td><td>--</td></tr>
* <tr><td>20</td><td>205</td><td>360</td><td>492</td><td>594</td><td>678</td><td>--</td></tr>
* <tr><td>21</td><td>228</td><td>402</td><td>550</td><td>667</td><td>765</td><td>--</td></tr>
* <tr><td>22</td><td>251</td><td>444</td><td>609</td><td>740</td><td>852</td><td>--</td></tr>
* <tr><td>23</td><td>276</td><td>490</td><td>675</td><td>818</td><td>950</td><td>--</td></tr>
* <tr><td>24</td><td>302</td><td>537</td><td>741</td><td>895</td><td>1,047</td><td>--</td></tr>
* <tr><td>25</td><td>330</td><td>588</td><td>812</td><td>986</td><td>1,153</td><td>--</td></tr>
* <tr><td>26</td><td>357</td><td>638</td><td>882</td><td>1,076</td><td>1,259</td><td>--</td></tr>
* <tr><td>27</td><td>387</td><td>693</td><td>961</td><td>1,172</td><td>1,374</td><td>--</td></tr>
* <tr><td>28</td><td>417</td><td>745</td><td>1,040</td><td>1,267</td><td>1,490</td><td>1,689</td></tr>
* <tr><td>29</td><td>448</td><td>807</td><td>1,122</td><td>1,368</td><td>1,616</td><td>1,844</td></tr>
* <tr><td>30</td><td>481</td><td>866</td><td>1,205</td><td>1,469</td><td>1,741</td><td>1,999</td></tr>
* <tr><td>31</td><td>516</td><td>930</td><td>1,298</td><td>1,588</td><td>1,874</td><td>2,160</td></tr>
* <tr><td>32</td><td>550</td><td>993</td><td>1,391</td><td>1,706</td><td>2,006</td><td>2,321</td></tr>
* <tr><td>33</td><td>587</td><td>1,061</td><td>1,488</td><td>1,827</td><td>2,150</td><td>2,495</td></tr>
* <tr><td>34</td><td>624</td><td>1,129</td><td>1,586</td><td>1,948</td><td>2,294</td><td>2,669</td></tr>
* <tr><td>35</td><td>663</td><td>1,204</td><td>1,692</td><td>2,080</td><td>2,454</td><td>2,846</td></tr>
* <tr><td>36</td><td>702</td><td>1,278</td><td>1,797</td><td>2,212</td><td>2,614</td><td>3,022</td></tr>
* <tr><td>37</td><td>744</td><td>1,355</td><td>1,912</td><td>2,352</td><td>2,788</td><td>3,219</td></tr>
* <tr><td>38</td><td>785</td><td>1,432</td><td>2,027</td><td>2,493</td><td>2,962</td><td>3,416</td></tr>
* <tr><td>39</td><td>828</td><td>1,515</td><td>2,144</td><td>2,644</td><td>3,130</td><td>3,614</td></tr>
* <tr><td>40</td><td>872</td><td>1,598</td><td>2,260</td><td>2,795</td><td>3,298</td><td>3,813</td></tr>
* </table>
*
* This behavior adds an float data member called "Merch Val" to trees that
* holds the value of the tree's timber.
*
* This class's namestring and parameter call string are both
* "MerchValueCalculator".
*
* This behavior may not be applied to seedlings.
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clMerchValueCalculator : virtual public clBehaviorBase {
  public:

  /**
  * Constructor.
  * @param p_oSimManager clSimManager object.
  */
  clMerchValueCalculator(clSimManager *p_oSimManager);

  /**
  * Destructor.  Deletes arrays.
  */
  ~clMerchValueCalculator();

  /**
  * Makes value calculations.  First, the values in the "Merchantable Timber
  * Value" grid are cleared.  Then a query is sent to the tree population to
  * get all trees to which this behavior is applied.  For each, the value is
  * calculated by GetTreeValue().  This value is placed in the "Merch Val"
  * float tree data member.  The species totals are put in the "Merchantable
  * Timber Value" grid.
  */
  void Action();

  /**
  * Does setup for this behavior.  Calls:
  * <ol>
  * <li>GetParameterFileData()</li>
  * <li>FormatQueryString()</li>
  * <li>PopulateTables()</li>
  * <li>SetupGrid()</li>
  * <li>Action() so that the initial conditions value will be added</li>
  * </ol>
  * @param p_oDoc DOM tree of parsed input file.
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
  * Registers the "Merch Val" float data member.  The return codes are captured
  * in the mp_iVolumeCodes array.
  * @throw modelErr if this behavior is being applied to any tree type except
  * saplings, adults, and snags.
  */
  void RegisterTreeDataMembers();

  protected:
  /**
   * Grid holding total values for each species.  The grid name is
   * "Merchantable Timber Value".  The grid contains only 1 grid cell.  It has
   * X float data members, where X = the total number of species.  The data
   * member names are "value_x", where "x" is the species number.
   */
  clGrid* mp_oValueGrid;

  /** Value per thousand board feet of merchantable timber, in whatever
  * currency the user prefers.  During parameter file reading this will be
  * divided by 1000 to be value per foot of timber. Array size is # total
  * species.*/
  float *mp_fVal;

  /** Form classes.  This is the proportion of DBH that the diameter is at the
  * top of the first log.  This is entered by the user as a percentage between
  * 60 and 100 but to save math we convert to a proportion and subtract it from
  * 1 so it's the inverse.  Array size is # behavior species.*/
  float *mp_fFormClass;

  /** Taper table.  This is the amount by which diameter is reduced,
  * in inches, at the top of the last log in a 2-, 3-, 4-, 5-, or 6-log tree.
  * First array index is 16 (m_iDBHIncs, DBH in 2-in increments from 10 to 40
  * inches); second index is 6 (m_iMaxLogs).*/
  float **mp_fTaperTable;

  /** Table of board feet for form class 78.  This is the volume, in board
   * feet, for trees having 1 to 6 logs.  First array index is m_iFormDBHIncs;
   * second index is m_iMaxLogs.*/
  float **mp_fFormClass78Table;

  /** Table of board feet for form class 79.  This is the volume, in board
   * feet, for trees having 1 to 6 logs.  First array index is m_iFormDBHIncs;
   * second index is m_iMaxLogs.*/
  float **mp_fFormClass79Table;

  /** Table of board feet for form class 80.  This is the volume, in board
   * feet, for trees having 1 to 6 logs.  First array index is m_iFormDBHIncs;
   * second index is m_iMaxLogs.*/
  float **mp_fFormClass80Table;

  /** Table of board feet for form class 81.  This is the volume, in board
   * feet, for trees having 1 to 6 logs.  First array index is m_iFormDBHIncs;
   * second index is m_iMaxLogs.*/
  float **mp_fFormClass81Table;

  /** Table of board feet for form class 84.  This is the volume, in board
   * feet, for trees having 1 to 6 logs.  First array index is m_iFormDBHIncs;
   * second index is m_iMaxLogs.*/
  float **mp_fFormClass84Table;

  /** Table of board feet for form class 85.  This is the volume, in board
   * feet, for trees having 1 to 6 logs.  First array index is m_iFormDBHIncs;
   * second index is m_iMaxLogs.*/
  float **mp_fFormClass85Table;

  /**String to pass to clTreePopulation::Find() in order to get the trees for
  * which to calculate volume.  This will instigate a species/type search for
  * all the species and types to which this behavior applies.*/
  char *m_cQuery;

  /**Holds data member codes for "Merch Val" float data member.  First array
  * index is total # species, second is number types (3 - sapling, adult,
  * snag)*/
  short int **mp_iMerchValCodes;

  /**
  * Holds data member codes for the "value_x" float data members of the
  * "Merchantable Timber Value" grid.  Array size is total # species.
  */
  short int *mp_iValueCodes;

  /** The maximum number of logs a tree can have.  Equal to 6. */
  short int m_iMaxLogs;

  /** Number of DBH increments in the taper table.  Equal to 16 (DBH in 2-in
  * increments from 10 to 40 inches).*/
  short int m_iDBHIncs;

  /** Number of DBH increments in the form class board feet tables.  Equal to
   * 31 (DBH in 1-in increments from 10 to 40 inches).*/
  short int m_iFormDBHIncs;

  /**Total number of species.  For the destructor.*/
  short int m_iNumTotalSpecies;

  /**
  * Gets the value for a tree.  First, GetNumLogs() is called to find out how
  * many 16-foot logs the tree has.  Then this uses the form class table for
  * the tree's species to determine the number of board feet in the tree.  It
  * determines the first array index by rounding down to the nearest integer
  * and subtracting 10 (with a max index of 30).  The second index is equal
  * to the number of logs minus 1.  This pair of indexes gets us the volume in
  * board feet of the tree from the form class table.  This volume is
  * multiplied by the value for the species and returned.
  *
  * If the dbh is less than the minimum, the value is 0.
  * @param fDBH SORTIE's DBH value, in in
  * @param iSpecies Tree's species.
  * @return Tree's value, in whatever currency units mp_fVal is in.
  */
  float GetTreeValue(const float &fDBH, const int &iSpecies);

  /**
  * Gets the number of logs in the merchantable bole height of a tree.
  *
  * This function calculates the number of logs in a tree.  It calculates the
  * taper to the top of the first log using the form class for the species.
  * After determining how much taper is left, it uses the taper table to
  * determine how many logs to add.  It determines the DBH increment index by
  * rounding down to the nearest even number and subtracting 10 (with a max
  * index of 15).  Then, for that index, it moves from 0 to m_iMaxLogs across
  * the table until it finds the greatest entry that is less than or equal to
  * the remaining taper.  That array index is equal to the number of logs to
  * add to the first one.  The maximum value is 6.
  *
  * This function does not weed out trees with a DBH of less than 10 inches.
  *
  * @param fDBH SORTIE's DBH value, in in
  * @param iSpecies Tree's species.
  * @return Number of 16-foot logs in a tree.
  */
  int GetNumLogs(const float &fDBH, const int &iSpecies);

  /**
  * Reads values from the parameter file.
  * @param p_oDoc DOM tree of parsed input file.
  * @param p_oPop Tree population object.
  * @throw modelErr if a form class value is not equal to 78, 79, 80, 81, 84,
  * or 85.
  */
  void GetParameterFileData(xercesc::DOMDocument *p_oDoc, clTreePopulation *p_oPop);

  /**
  * Formats the string in m_cQuery.  This value will be used in Action() to
  * pass to clTreePopulation::Find() in order to get the trees to act on.
  * @param p_oPop Tree population object.
  */
  void FormatQueryString(clTreePopulation *p_oPop);

  /**
  * Populates the taper and form class board feet tables.  The table arrays are
  * declared and the values populated according to the values in the
  * documentation above.
  */
  void PopulateTables();

  /**
  * Sets up the "Merchantable Timber Value" grid.  This ignores any maps.
  */
  void SetupGrid();
};
//---------------------------------------------------------------------------
#endif // VolumeCalculator_H

#!/bin/sh
. /lib/functions.sh

# Usage: 
	# Place this file webfilter-metadata.sh  at /usr/bin/webfilter-metadata.sh.
	# Run below command to generate wf metadata xml file(filename given as argument 1)
	# webfilter-metadata.sh /etc/confd/cdb/webfilter-meta-data_init.xml

#/etc/confd/cdb/webfilter-meta-data_init.xml
CDBFILEPATH=/tmp
WFMETADATADEFFILENAME=$CDBFILEPATH/webfilter-meta-data_init.xml

category_fun() {
  local name=$1

  echo "      <category>" >> $WFMETADATAFILENAME
  config_get val "$name" id
  echo "        <id>$val</id>" >> $WFMETADATAFILENAME

#  echo "        <applications>" >> $WFMETADATAFILENAME
#  config_list_foreach "$name" application handle_application
#  echo "        </applications>" >> $WFMETADATAFILENAME

  config_get p "$name" parent
  echo "        <parent>$p</parent>" >> $WFMETADATAFILENAME

  config_get val "$name" name 
  x=`echo $val | sed "s/\&/\&amp;/g"`
  echo "        <name>$x</name>" >> $WFMETADATAFILENAME

  config_get d "$name" description 
  echo "        <description>$d</description>" >> $WFMETADATAFILENAME

  echo "      </category>" >> $WFMETADATAFILENAME
}

group_fun() {
  local name=$1

  echo "      <group>" >> $WFMETADATAFILENAME

  config_get val "$name" id
  echo "        <id>$val</id>" >> $WFMETADATAFILENAME    

  config_get val "$name" name
  x=`echo $val | sed "s/\&/\&amp;/g"`      
  echo "        <name>$x</name>" >> $WFMETADATAFILENAME
  echo "        <categories>" >> $WFMETADATAFILENAME
  config_list_foreach "$name" category handle_category
  echo "        </categories>" >> $WFMETADATAFILENAME

  echo "      </group>" >> $WFMETADATAFILENAME
}

application_fun() {
  local name=$1

  echo "      <application>" >> $WFMETADATAFILENAME
  config_get val "$name" id
  echo "        <id>$val</id>" >> $WFMETADATAFILENAME
  
  config_get val "$name" name 
  x=`echo $val | sed "s/\&/\&amp;/g"`      
  echo "        <name>$x</name>" >> $WFMETADATAFILENAME
  config_list_foreach "$name" behaviour handle_behaviour

  echo "      </application>" >> $WFMETADATAFILENAME
}

behaviour_fun() {
  local name=$1
  echo "      <behaviour>" >> $WFMETADATAFILENAME
  config_get val "$name" id
  echo "        <id>$val</id>">> $WFMETADATAFILENAME
  
  config_get val "$name" name 
  x=`echo $val | sed "s/\&/\&amp;/g"`      
  echo "        <name>$x</name>" >> $WFMETADATAFILENAME
  echo "        <description></description>">> $WFMETADATAFILENAME

  echo "      </behaviour>" >> $WFMETADATAFILENAME
}

handle_application() {
  echo "          <application>$1</application>" >> $WFMETADATAFILENAME
}

handle_category() {
  echo "          <categores>$1</categores>" >> $WFMETADATAFILENAME
}

handle_behaviour() {
  echo "        <behaviors>$1</behaviors>" >> $WFMETADATAFILENAME
}

write_filter_level() {
  echo "    <filter-levels>" >> $WFMETADATAFILENAME

  echo "      <filter-level>" >> $WFMETADATAFILENAME

  echo "        <level>high</level>" >> $WFMETADATAFILENAME
  echo "           <categories>10</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>11</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>15</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>19</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>27</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>32</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>33</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>36</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>43</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>44</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>46</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>47</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>48</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>49</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>52</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>53</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>54</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>55</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>56</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>57</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>58</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>59</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>62</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>67</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>71</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>73</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>76</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>8</categories>" >> $WFMETADATAFILENAME
  echo "        </filter-level>" >> $WFMETADATAFILENAME

  echo "      <filter-level>" >> $WFMETADATAFILENAME
  echo "        <level>low</level>" >> $WFMETADATAFILENAME
  echo "           <categories>15</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>56</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>57</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>58</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>59</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>67</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>71</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>73</categories>" >> $WFMETADATAFILENAME
  echo "        </filter-level>" >> $WFMETADATAFILENAME

  echo "      <filter-level>" >> $WFMETADATAFILENAME
  echo "        <level>med</level>" >> $WFMETADATAFILENAME
  echo "           <categories>10</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>11</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>15</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>27</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>32</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>43</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>44</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>46</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>49</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>54</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>56</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>57</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>58</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>59</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>62</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>67</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>71</categories>" >> $WFMETADATAFILENAME
  echo "           <categories>73</categories>" >> $WFMETADATAFILENAME
  echo "        </filter-level>" >> $WFMETADATAFILENAME

  echo "    </filter-levels>" >> $WFMETADATAFILENAME
}

create_wf_metadata_xml_file() {
# $1, argument 1(input): UCI module/file name from which application mapping can be read

  local appuciname=$1

  rm -f $WFMETADATAFILENAME
  echo "<config xmlns=\"http://tail-f.com/ns/config/1.0\">" > $WFMETADATAFILENAME
  #echo "  <webfilter-meta-data xmlns=\"urn:ciscosb:params:xml:ns:yang:ciscosb-security-common\">" >> $WFMETADATAFILENAME
  echo "  <webfilter-meta-data xmlns=\"http://cisco.com/ns/ciscosb/security-common\">" >> $WFMETADATAFILENAME

  config_load $appuciname

  echo "    <categories>" >> $WFMETADATAFILENAME
  config_foreach category_fun category $appuciname
  echo "    </categories>" >> $WFMETADATAFILENAME

  echo "    <groups>" >> $WFMETADATAFILENAME
  config_foreach group_fun group $appuciname 
  echo "    </groups>" >> $WFMETADATAFILENAME
  
#  echo "    <applications>" >> $WFMETADATAFILENAME
#  config_foreach application_fun application $appuciname
#  echo "    </applications>" >> $WFMETADATAFILENAME

#  echo "    <behaviours>" >> $WFMETADATAFILENAME
#  config_foreach behaviour_fun behaviour $appuciname
#  echo "    </behaviours>" >> $WFMETADATAFILENAME

  write_filter_level
  echo "  </webfilter-meta-data>" >> $WFMETADATAFILENAME
  echo "</config>" >> $WFMETADATAFILENAME
}

WFMETADATAFILENAME=$WFMETADATADEFFILENAME
if [ "$1" != "" ]; then
  WFMETADATAFILENAME=$1
fi

create_wf_metadata_xml_file application

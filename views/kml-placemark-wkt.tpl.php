<?php
// $Id: kml-placemark.tpl.php,v 1.1.2.1 2010/06/03 14:14:09 tmcw Exp $
?><Placemark>
  <name>
    <![CDATA[<?php print $name ?>]]>
  </name>
  <description>
    <![CDATA[<?php print $description ?>]]>
  </description>
  <?php if ($styleURL): ?>
  <styleUrl><?php echo $styleURL; ?></styleUrl>
  <?php endif; ?>
  <?php print $geometry ?>
</Placemark>

<Placemark>
  <name>
    <![CDATA[<?php print strip_tags($name) ?>]]>
  </name>
  <description>
    <![CDATA[<?php print $description ?>]]>
  </description>
  <?php if ($styleURL): ?>
    <styleUrl><?php echo $styleURL; ?></styleUrl>
  <?php endif; ?>
  <?php print $geometry ?>
</Placemark>

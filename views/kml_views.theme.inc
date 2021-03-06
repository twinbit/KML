<?php
// $Id: kml_views.theme.inc,v 1.1.2.2 2010/06/03 14:14:09 tmcw Exp $

/**
 * @file
 * Theming functions for KML module views output.
 */

function template_preprocess_views_view_kml(&$vars) {
  $view = $vars['view'];
  
  $points = $view->style_plugin->map_rows($vars['rows']);
  $styles = '';
  
  if ($vars['options']['scaledpoints']['enable'] && $points['scale_ramp']) {
    foreach ($points['scale_ramp'] as $scale) {
      $styles .= theme('kml_style', '1', $scale, $vars['options']);
    }
  }
  
  foreach ($points as $point) {
    if ($vars['options']['data_source']['provider'] == 'openlayers_wkt' && $point['wkt']) {
      $rows .= theme('kml_placemark_wkt', $point, $points);
    }
    elseif ($vars['options']['data_source']['provider'] == 'lat_lon' && $point['point']) {
      $rows .= theme('kml_placemark', $point, $points);
    }
  }
  
  $vars['rows'] = $rows;
  $vars['style'] = $styles;
  $vars['viewtitle'] = $view->name;  
  $filename = $vars['viewtitle'] .'.kml'; 
    
  drupal_set_header('Content-Type: application/vnd.google-earth.kml+xml');
  drupal_set_header("Content-Disposition: attachment; filename=$filename");
}


/**
 * Internal preprocess for theme('kml_node_url').
 */
function _kml_preprocess_node_url(&$vars) {
  if (($vars['point']['node_nid']) && ($vars['point']['node_url_title'])) {
    $vars['url'] = theme('kml_node_url', $vars['point']['node_nid'], $vars['point']['node_url_title']);
    $vars['name'] = strip_tags($vars['name']);
    $vars['description'] = $vars['url'] . $vars['description'];
  }
}

/**
 * Preprocess for theme('kml_placemark').
 */
function template_preprocess_kml_placemark(&$vars) {
  $vars['name'] = filter_xss_admin($vars['point']['name']);
  $vars['description'] = filter_xss_admin($vars['point']['description']);
  $vars['coords'] = check_plain($vars['point']['point']);
  $vars['styleURL'] = check_plain($vars['point']['styleURL']);
  _kml_preprocess_node_url($vars);
}

/**
 * Preprocess for theme('kml_placemark_wkt').
 */
function template_preprocess_kml_placemark_wkt(&$vars) {
  $vars['name'] = filter_xss_admin($vars['point']['name']);
  $vars['description'] = filter_xss_admin($vars['point']['description']);
  $vars['geometry'] = $vars['point']['wkt'];
  $vars['styleURL'] = check_plain($vars['point']['styleURL']);
  _kml_preprocess_node_url($vars);
}

/**
 * Preprocess for theme('kml_node_url').
 */
function template_preprocess_kml_node_url(&$vars) {
  $vars['nid'] = $vars['nid'];
  $vars['title'] = $vars['title'];
}

/**
 * Preprocess for theme('kml_style').
 */
function template_preprocess_kml_style(&$vars) {  
  $vars['iconurl'] = $vars['options']['scaledpoints']['iconurl'];
}

/**
 * Theme function for kml feed icon.
 */
function theme_kml_feed_icon($url, $title, $icon) {
  if ($image = theme('image', $icon, t('Download KML Feed'), $title)) {
    return '<a href="'. check_url($url) 
      .'" class="feed-icon">'. $image .'</a>';
  }
}


/**
 * Theme function for kml feed image
 */
function theme_kml_feed_image() {
  $icon = drupal_get_path('module', 'kml').'/images/kml.png';
  return theme('image', $icon, t('Download KML Feed'));  
}
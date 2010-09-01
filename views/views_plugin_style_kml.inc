<?php
// $Id: views_plugin_style_kml.inc,v 1.1.2.2 2010/05/12 14:22:43 rsoden Exp $

/**
 * @file
 * Extending the view_plugin_style class to provide a kml view style.
 */
class views_plugin_style_kml extends views_plugin_style {

  /**
   * Initialize plugin.
   *
   * Set feed image for shared rendering later.
   */
  function init(&$view, &$display, $options = NULL) {
    parent::init($view, $display, $options = NULL);
    $this->feed_image = drupal_get_path('module', 'views_kml') . 
      '/images/kml.png';
  }

  /**
   * Attach this view to another display as a feed.
   *
   * Provide basic functionality for all export style views like attaching a
   * feed image link.
   */
  function attach_to($display_id, $path, $title) {
    $url_options = array('html' => TRUE);
    $image = theme('image', $this->feed_image);
    $this->view->feed_icon .= l($image, $path, $url_options);
  }

  function option_definition() {
    $options = parent::option_definition();
    
    $field_options = array(
      'name' => t('Name'),
      'description' => t('Description'),
      'longitude' => t('Longitude'),
      'latitude' => t('Latitude'),
    );
    
    foreach ($field_options as $k => $v) {
      $fields[$k] = array('default' => '');
    }
    
    $options['fields'] = array(
      'contains' => $fields,
    );
    
    $options['scaledpoints']['enable'] = array();
    $options['scaledpoints']['iconurl'] = array();
    $options['scaledpoints']['valuefield'] = array();
    $options['scladepoints']['ranges'] = array(
      'contains' => $ranges,
    );
    
    return $options;
  }


  /**
   * Provide a form for setting options.
   *
   * @param array $form
   * @param array $form_state
   */
  function options_form(&$form, &$form_state) { 
    parent::options_form($form, $form_state);                  
    $options = parent::option_definition();

    $handlers = $this->display->handler->get_handlers('field');
    if (empty($handlers)) {
      $form['error_markup'] = array(
        '#value' => t('You need at least one field before you can 
        configure your field settings'),
        '#prefix' => '<div class="error form-item description">',
        '#suffix' => '</div>',
      );
    }
    else {
      $field_names[$field] = array('' => '--');
      foreach ($handlers as $field => $handler) {
        if ($label = $handler->label()) {
          $field_names[$field] = $label;
        }
        else {
          $field_names[$field] = $handler->ui_name();
        }
      }
      $field_options = array(
        'name' => t('Name'),
        'description' => t('Description'),
        'longitude' => t('Longitude'),
        'latitude' => t('Latitude'),
      );
      $form['fields'] = array(
        '#type' => 'fieldset',
        '#title' => 'Field usage',
        '#description' => t('Select the fields that contain the latitude,
        longitude and title of each point. 
        Remaining fields will be available in the 
        "content" region of the point.'),
        '#weight' => -10,
      );
      foreach ($field_options as $k => $v) {
        $form['fields'][$k] = array(
          '#type' => 'select',
          '#title' => $v,
          '#options' => $field_names,
          '#default_value' => $this->options['fields'][$k],
          '#required' => ($k == 'class' ? FALSE : TRUE),
        );
      }
      $form['scaledpoints']['enable'] = array(
       '#type' => 'checkbox',
       '#title' => t('Use Scaled Points'),
       '#id' => 'scaledpoints-enable',
       '#default_value' => $this->options['scaledpoints']['enable'],
      );
      $form['scaledpoints']['iconurl'] = array(
        '#type' => 'textfield',
        '#title' => t('Path to Icon'),
        '#description' => t("This should be either absolute or relative to the path of this view's KML display"),
        '#process' => array('views_process_dependency'),
        '#dependency' => array('scaledpoints-enable' => array('1')),
        '#default_value' => $this->options['scaledpoints']['iconurl'],
      );
      $form['scaledpoints']['valuefield'] = array(
        '#type' => 'select',
        '#title' => t('Value Field to Use for Scaling'),
        '#options' => $field_names,
        '#process' => array('views_process_dependency'),
        '#dependency' => array('scaledpoints-enable' => array('1')),
        '#default_value' => $this->options['scaledpoints']['valuefield'],
      );
      $ranges = array(
        '3' => '3',
        '4' => '4',
        '5' => '5',
        '6' => '6',
      );
      $form['scaledpoints']['ranges'] = array(
        '#type' => 'select',
        '#title' => t('Number of Ranges'),
        '#options' => $ranges,
        '#process' => array('views_process_dependency'),
        '#dependency' => array('scaledpoints-enable' => array('1')),
        '#default_value' => $this->options['scaledpoints']['ranges'],
      );
    }
  }

  function map_rows($rows) {
    // Fields must be rendered in order as of Views 2.3, 
    // so we will pre-render everything.
    $renders = array();
    $keys = array_keys($this->view->field);

    foreach ($rows as $count => $row) {
      foreach ($keys as $id) {
        $renders[$count][$id] = $this->view->field[$id]->theme($row);
      }
    }  

    $points = array();
    foreach ($renders as $id => $row) {
      $point = array();
      foreach ($this->view->field as $key => $field) {
        if ($key == $this->options['fields']['name']) {
          $point['name'] = $row[$key];
        }
        elseif ($key == $this->options['fields']['description']) {
          $point['description'] = $row[$key];
        }
        elseif ($key == $this->options['fields']['longitude']) {
          $point['lon'] = $row[$key];
        }
        elseif ($key == $this->options['fields']['latitude']) {
          $point['lat'] = $row[$key];
        }
        elseif ($key == $this->options['scaledpoints']['valuefield']) {
          $point['scalevalue'] = (float) str_replace(',' ,'', $row[$key]);
          $values[] = (float) str_replace(',' ,'', $row[$key]);
        }
      }

      $point['point'] = $point['lon'] . ',' . $point['lat'] . ',0';
      unset($point['lat']);
      unset($point['lon']);

      if ($this->options['linestring']['enable']) {
        $points[$point['group']][] = $point;
      }
      else {
        $points[] = $point;
      }
    }
    
    if ($this->options['scaledpoints']['enable']) {
      include_once('scaled_points.inc');
      
      $value_min = min($values);
      $value_max = max($values);
      $value_range = $value_max - $value_min;
      
      $value_ramp = stylewriter_color_ramp_grad(
        $value_min, 
        $value_max, 
        $this->options['scaledpoints']['ranges']
      );
      
      $scale_ramp = stylewriter_color_ramp_grad(
        '0.2', 
        '1', 
        $this->options['scaledpoints']['ranges']
      );
      
      foreach ($points as $i => $point) {
       $rank = ($point['scalevalue'] - $value_min) / $value_range;
       $ramp_idx = ($rank == 1) ? 
         count($scale_ramp) - 1 : 
         floor($rank * $this->options['scaledpoints']['ranges']);
       $points[$i]['scale'] = $scale_ramp[$ramp_idx];
       $points[$i]['styleURL'] = $ramp_idx + 1;
      }
      
      $points['scale_ramp'] = $scale_ramp;
    }

    return $points;
  }
}
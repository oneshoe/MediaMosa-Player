<?php

/**
 * @file
 * This file contains all the theme function that belong to the Fideo module.
 *
 * @author One Shoe
 */

/**
 * Theme function for rendering a track element.
 *
 * @param array $variables
 */
function theme_fideo_track($variables) {
  $element = $variables['element'];
  $attributes = array(
    'kind' => $element['#type'],
    'src' => $element['#file'],
    'srclang' => $element['#lang'],
    'label' => $element['#title'],
  );
  if (!empty($element['#mimetype'])) $attributes['type'] = $element['#mimetype'];
  $attributes = array_merge($attributes, $element['#attributes']);

  return '<track'. drupal_attributes($attributes)." />\n";
}

/**
 * Theme function for rendering a source element.
 *
 * @param array $variables
 */
function theme_fideo_source($variables) {
  $element = $variables['element'];
  $attributes = array(
    'src' => $element['#file'],
  );

  // Manually add the 'type' attribute as it contains quotes and should not be
  // check_plained() the normal way.
  unset($element['#attributes']['src']);
  $attributes = array_merge($attributes, $element['#attributes']);
  unset($attributes['type']);

  $output = '<source'. drupal_attributes($attributes);
  if (!empty($element['#mimetype'])) {
    $output .= ' type=\''. check_plain($element['#mimetype']);
    if (!empty($element['#codecs'])) {
      $output .= '; codecs="'. check_plain($element['#codecs']) .'"';
    }
    $output .= '\'';
  }
  $output .= " />\n";

  return $output;
}

/**
 * Theme function for rendering the video container.
 *
 * @param array $variables
 */
function theme_fideo_video($variables) {
  $element = $variables['element'];
  static $counter = 0, $assets_added = FALSE;
  // Update the counter even if the video element has its own ID.
  $counter++;

  if (!$assets_added && empty($element['#skip_assets'])) {
    $path = drupal_get_path('module', 'fideo') .'/assets';
    drupal_add_js($path .'/video.js');
    drupal_add_js($path .'/fideo.js');
    drupal_add_css($path .'/video-js.css', array('media' => 'screen'));
    drupal_add_css($path .'/video.css', array('media' => 'screen'));
    $assets_added = TRUE;
  }

  $attributes = array(
    'id' => (!empty($element['#id']) ? $element['#id'] : 'fideo-video-'. ($counter)),
  );
  $element['#id'] = $attributes['id'];
  if (!empty($element['#disabled'])) {
    $attributes['disabled'] = 'disabled';
  }
  $attributes = array_merge($attributes, $element['#attributes']);
  $attributes['class'] = (!empty($attributes['class']) ? $attributes['class']. ' ' : '') .'video-js';

  $js = array('type' => 'video');
  if (!empty($element['#multistream'])) {
    // If #multistream is set, it should contain a CSS selector.
    $js['multi'] = $element['#multistream'];
  }

  $js['ratio'] = $attributes['height'] / $attributes['width'];
  //$attributes['width'] = '100%';
  //$attributes['height'] = '100%';
  //unset($attributes['width']);
  //unset($attributes['height']);

  drupal_add_js(array('fideo' => array($element['#id'] => $js)), 'setting');

  if (!empty($element['#poster'])) {
    $attributes['poster'] = $element['#poster'];
  }
  $element['#attributes'] = $attributes;

  foreach (array('autoplay', 'controls', 'preload') as $key) {
    if (isset($element['#'. $key]) && $element['#'. $key] !== FALSE) {
      $attributes[$key] = ($element['#'. $key] === TRUE ? $key : $element['#'. $key]);
    }
  }

  $output = '<div class="fideo-item" id="fideo-'. check_plain($element['#id']) .'"><div class="video-js-box">' ."\n";
  $output .= '<video'. drupal_attributes($attributes) .">\n";
  $output .= $element['#children'];
  if (!empty($element['#flash_fallback'])) {
    $output .= theme('fideo_fallback', array('element' => $element));
  }
  $output .= "</video>\n</div>\n</div>";

  return $output;
}

/**
 * Theme function for rendering a container width slide images.
 *
 * @param array $variables
 */
function theme_fideo_slides($variables) {
  $element = $variables['element'];
  static $counter = 0;
  $counter++;

  $element['#attributes']['id'] = (!empty($element['#id']) ? $element['#id'] : 'fideo_slides_'. ($counter));
  $element['#id'] = $element['#attributes']['id'];

  $js = array('type' => 'slides');
  // Build the javascript information.
  if (!empty($element['#target'])) {
    $js['target'] = $element['#target'];
    $timecodes = array();
    foreach (element_children($element) as $child) {
      if (!empty($element[$child]['#type']) && $element[$child]['#type'] == 'slide') {
        $timecodes[] = $element[$child]['#start'];
      }
      $js['timecodes'] = $timecodes;
    }
  }

  // Add the configuration to javascript.
  drupal_add_js(array('fideo' => array($element['#id'] => $js)), 'setting');

  $element['#attributes']['class'] = (!empty($element['#attributes']['class']) ? $element['#attributes']['class'] .' ' : ''). 'fideo-slides';
  $output = '<div'. drupal_attributes($element['#attributes']) .'>'. $element['#children'] .'</div>';
  return $output;
}

/**
 * Theme function for rendering a single slide item.
 *
 * @param array $variables
 */
function theme_fideo_slide($variables) {
  $element = $variables['element'];
  $element['#attributes']['class'] = (!empty($element['#attributes']['class']) ? $element['#attributes']['class'] .' ' : ''). 'fideo-slide';
  $output = '<div'. drupal_attributes($element['#attributes']) .'>';
  if (!empty($element['#file'])) {
    $element['#alt'] = isset($element['#alt']) ? $element['#alt'] : '';
    $output .= theme('image', array('path' => $element['#file'], 'alt' => $element['#alt'], 'title' => $element['#alt']));
  }

  if (isset($element['#children'])) {
    $output .= $element['#children'];
  }

  $output .= '</div>';

  return $output;
}

/**
 * Theme function for rendering the fallback player.
 *
 * @param array $variables
 */
function theme_fideo_fallback($variables) {
  $element = $variables['element'];
  $output = '';

  module_load_include('inc', 'fideo', 'fideo.render');

  // Try to find an MP4 version for adding the Flash player fallback.
  if ($keys = _fideo_element_by_type($element, 'source', '#mimetype', 'video/mp4')) {
    // Use the first-found mp4 element.
    $source = $element[$keys[0]];
    $player_swf = base_path() . variable_get('fideo_flowplayer_src', drupal_get_path('module', 'fideo') . FIDEO_DEFAULT_FLOWPLAYER_SRC);

    $attributes = array(
      'id' => $element['#id'] .'-flash',
      'class' => 'vjs-flash-fallback',
      'width' => '100%',
      'height' => '100%',
      'type' => 'application/x-shockwave-flash',
      'data' => $player_swf,
    );

    $playlist = array();
    if (!empty($element['#poster'])) {
      $playlist[] = '"'. check_plain(addslashes($element['#poster'])) .'"';
    }
    $item = '{"url": "'. check_plain(addslashes($source['#file'])) .'","autoBuffering":true,"autoPlay":'. ($element['#autoplay'] ? 'true' : 'false');
    $item .= '}';

    $playlist[] = $item;

    // Include the fideo.render.inc file so we are able to use the flashvar
    // helper function.
    module_load_include('inc', 'fideo', 'fideo.render');

    $output .= '<object'. drupal_attributes($attributes) .">\n";
    $output .= '<param name="movie" value="'. check_plain($player_swf) .'" />';
    $output .= '<param name="allowfullscreen" value="true" />';
    $output .= '<param name="flashvars" value=\''. (_fideo_fallback_flashvars($element, $source)) .'\' />' ."\n";
    if (!empty($element['#poster'])) {

      $output .= theme('image', array(
        'path' => $element['#poster'],
        'width' => check_plain($attributes['width']),
        'height' => check_plain($attributes['height']),
        'alt' => t('Unable to play video.')
      ));

    }
    $output .= "</object>\n";
  }

  return $output;
}

/**
 * Theme function for rendering HTML markers.
 *
 * @param array $variables
 */
function theme_fideo_markers($variables) {
  $element = $variables['element'];

  $element['#attributes']['id'] = !empty($element['#id']) ? $element['#id'] : 'fideo_markers';

  $js = array('fideo_markers' => array('type' => 'markers'));

  if ($element['#target']) {
    $js['fideo_markers']['target'] = $element['#target'];
  }

  // Add required js files.
  $path = drupal_get_path('module', 'fideo');

  drupal_add_js(base_path() . variable_get('fideo_qtip_src', drupal_get_path('module', 'fideo') . FIDEO_DEFAULT_QTIP_SRC));
  drupal_add_js(array('fideo' => $js), 'setting');

  return '<div '. drupal_attributes($element["#attributes"]) .'>'. $element['#children'] .'</div>';
}

/**
 * Theme function for rendering a timecode marker
 *
 * @param array $variables
 */
function theme_fideo_marker($variables) {
  $element = $variables['element'];

  // Create an array item where we can store the class names.
  if (!isset($element['#attributes']['class'])) {
    $element['#attributes']['class'] = '';
  }

  // Add class to attributes
  $element['#attributes']['class'] .= (isset($element['#attributes']['class'])) ? ' timecode-marker' : 'timecode-marker';
  //$element['#attributes']['title'] = $element['#value'];

  $path = !drupal_is_front_page() ? $_GET['q'] : $element['#value'];

  // @todo Later on, change the start and end times of these markers to the new Media Fragments format.
  return l($element['#value'], $path, array('fragment' => 'start:'. $element['#start'] .':end:'. $element['#end'], 'attributes' => array('class' => array($element['#attributes']['class'])), 'html' => TRUE));
}

/**
 * Theme function for Fideo markup.
 *
 * @param array $variables
 *
 * @return array
 */
function theme_fideo_markup($variables) {
  $element = $variables['element'];
  if (!isset($element['#value']) && isset($element['#children'])) {
    $element['#value'] = $element['#children'];
  }
  $output = $element['#value'];
  return $output;
}

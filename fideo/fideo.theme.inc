<?php
// $Id: fideo.theme.inc 109 2011-06-07 14:39:55Z thijs $

/**
 * @file
 * Theme functions for the fideo module.
 */

/**
 * Theme function for rendering a track element.
 */
function theme_fideo_track($element) {
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
 */
function theme_fideo_source($element) {
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
 */
function theme_fideo_video($element) {
  static $counter = 0, $assets_added = FALSE;
  // Update the counter even if the video element has its own ID.
  $counter++;

  if (!$assets_added) {
    $path = drupal_get_path('module', 'fideo') .'/assets';
    drupal_add_js($path .'/video.js');
    drupal_add_js($path .'/fideo.js');
    drupal_add_css($path .'/video-js.css', 'module', 'screen');
    drupal_add_css($path .'/video.css', 'module', 'screen');
    $assets_added = TRUE;
  }

  $attributes = array(
    'id' => (!empty($element['#id']) ? $element['#id'] : 'fideo_video_'. ($counter)),
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

  $output = '<div class="fideo-item" id="fideo-c'. check_plain($element['#id']) .'"><div class="video-js-box">' ."\n";
  $output .= '<video'. drupal_attributes($attributes) .">\n";
  $output .= $element['#children'];
  $output .= theme('fideo_fallback', $element);
  $output .= "</video>\n</div>\n</div>";

  return $output;
}

/**
 * Theme function for rendering a container width slide images.
 */
function theme_fideo_slides($element) {
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
 */
function theme_fideo_slide($element) {
  $element['#attributes']['class'] = (!empty($element['#attributes']['class']) ? $element['#attributes']['class'] .' ' : ''). 'fideo-slide';
  $output = '<div'. drupal_attributes($element['#attributes']) .'>';
  if (!empty($element['#file'])) {
    $alt = _fideo_check_plain($element['#alt']);
    $output .= '<img src="'. check_plain($element['#file']) .'" alt="'. $alt .'" title="'. $alt .'" />';
  }
  $output .= $element['#children'] .'</div>';

  return $output;
}

/**
 * Theme function for rendering the fallback player.
 */
function theme_fideo_fallback($element) {
  $output = '';
  // Try to find an MP4 version for adding the Flash player fallback.
  if ($keys = _fideo_element_by_type($element, 'source', '#mimetype', 'video/mp4')) {
    // Use the first-found mp4 element.
    $source = $element[$keys[0]];
    $player_swf = url(drupal_get_path('module', 'fideo') .'/assets/flowplayer/flowplayer-3.2.5.swf');

    $attributes = array(
      'id' => $element['#id'] .'-flash',
      'class' => 'vjs-flash-fallback',
      'width' => $element['#attributes']['width'],
      'height' => $element['#attributes']['height'],
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

    $output .= '<object'. drupal_attributes($attributes) .">\n";
    $output .= '<param name="movie" value="'. check_plain($player_swf) .'" />';
    $output .= '<param name="allowfullscreen" value="true" />';
    $output .= '<param name="flashvars" value=\''. (_fideo_fallback_flashvars($element, $source)) .'\' />' ."\n";
    if (!empty($element['#poster'])) {
      $output .= '<img src="'. check_plain($element['#poster']) .'" width="'. check_plain($attributes['width']) .'" height="'. check_plain($attributes['height']) .'" alt="'. t('Unable to play video.') .'" />';
    }
    $output .= "</object>\n";
  }

  return $output;
}

/**
 * Theme function for rendering HTML markers.
 */
function theme_fideo_markers($element) {
  $element['#attributes']['id'] = !empty($element['#id']) ? $element['#id'] : 'fideo_markers';

  $js = array('fideo_markers' => array('type' => 'markers'));

  if ($element['#target']) {
    $js['fideo_markers']['target'] = $element['#target'];
  }

  // Add required js files.
  $path = drupal_get_path('module', 'fideo');
  drupal_add_js($path .'/assets/qtip/jquery.qtip-1.0.0-rc3.min.js');
  drupal_add_js(array('fideo' => $js), 'setting');

  return '<div '. drupal_attributes($element["#attributes"]) .'>'. $element['#children'] .'</div>';
}

/**
 * Theme function for rendering a timecode marker
 */
function theme_fideo_marker($element) {
  // Add class to attributes
  $element['#attributes']['class'] .= ($element['#attributes']['class']) ? ' timecode-marker' : 'timecode-marker';
  //$element['#attributes']['title'] = $element['#value'];
  return l((!drupal_is_front_page()) ? $_GET['q'] : $element['#value'], $path, array('fragment' => 'start:'. $element['#start'] .':end:'. $element['#end'], 'attributes' => $element['#attributes'], 'html' => TRUE));
}

/**
 * Helper function for building the flashvars for the fallback player embed code.
 */
function _fideo_fallback_flashvars($element, $source) {
  $config = array();
  $plugins = array();
  $playlist = array();

  // Build up the playlist.
  if (!empty($element['#poster'])) {
    // Add the poster as first item in the playlist array.
    $playlist[] = $element['#poster'];
  }

  // Main video item.
  $clip = array(
    'url' => $source['#file'],
    'autoBuffering' => TRUE,
    'autoPlay' => ($element['#autoplay'] ? TRUE : FALSE),
  );

  if ($tracks = _fideo_element_by_type($element, array('captions', 'subtitles', 'track'))) {
    // Find captions or subtitles, and use only the first one. Beware that
    // Flowplayer only supports *.srt subtitles.

    $clip['captionUrl'] = $element[$tracks[0]]['#file'];

    $plugins['captions'] = array(
      'url' => 'flowplayer.captions-3.2.2.swf',
      'captionTarget' => 'content',
    );
    $plugins['content'] = array(
      'url' => 'flowplayer.content-3.2.0.swf',
      'bottom' => 5,
      'height' => 40,
      'backgroundColor' => 'transparent',
      'backgroundGradient' => 'none',
      'border' => 0,
      'textDecoration' => 'outline',
      'style' => array(
        'body' => array('fontSize' => 18, 'fontFamily' => 'Helvetica,Arial,sans-serif', 'textAlign' => 'center', 'color' => '#ffffff'),
      ),
    );
  }

  $playlist[] = $clip;

  $config['playlist'] = $playlist;
  if (!empty($plugins)) {
    $config['plugins'] = $plugins;
  }

  $output = 'config='. drupal_to_js($config);
  return $output;
}

/**
 * Helper function for cleaning up a string that is about to be used in
 * a javascript scope.
 *
 * The workings of this function are based on the drupal_to_js() function with
 * the exception that it doesn't escape single quotes (') as the string will be
 * wrapped in double quotes.
 */
function _fideo_clean_js_string($str) {
  return str_replace(array("\r", "\n", "<", ">", "&"),
    array('\r', '\n', '\x3c', '\x3e', '\x26'),
    addcslashes($str, '"\\'));
}

/**
 * Helper function for performing a check_plain() for HTML attributes, which
 * also encodes the newline characters.
 */
function _fideo_check_plain($str) {
  $str = check_plain($str);
  return $str;
}

function theme_fideo_markup($element) {
  $output = $element['#value'];
  return $output;
}

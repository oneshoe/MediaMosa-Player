<?php
// $Id: fideo.render.inc 286 2011-12-13 12:55:28Z thijs $

/**
 * @file
 * This file contains all the functions that have impact on the video rendering
 * process.
 *
 * @author One Shoe
 */

/**
 * Helper function that will render the video element.
 * @see fideo_render() inside fideo.module.
 *
 * @param array $elements
 *
 * @return rendered video
 */
function _fideo_render(&$elements) {
  if (!empty($elements['#type']) && ($info = _fideo_element_info($elements['#type']))) {
    $elements += $info;
  }

  // Make any final changes to the element before it is rendered. This means
  // that the $element or the children can be altered or corrected before the
  // element is rendered into the final text.
  if (isset($elements['#pre_render'])) {
    foreach ($elements['#pre_render'] as $function) {
      if (function_exists($function)) {
        $elements = $function($elements);
      }
    }
  }

  $content = '';

  if (!isset($elements['#children'])) {
    $children = element_children($elements);
    // Render all the children that use a theme function.
    if (isset($elements['#theme']) && empty($elements['#theme_used'])) {
      $elements['#theme_used'] = TRUE;

      $previous = array();
      foreach (array('#value', '#type', '#prefix', '#suffix') as $key) {
        $previous[$key] = isset($elements[$key]) ? $elements[$key] : NULL;
      }
      // If we rendered a single element, then we will skip the renderer.
      if (empty($children)) {
        $elements['#printed'] = TRUE;
      }
      else {
        $elements['#value'] = '';
      }
      $elements['#type'] = 'markup';

      unset($elements['#prefix'], $elements['#suffix']);

      $content = theme($elements['#theme'], array('element' => $elements));

      foreach (array('#value', '#type', '#prefix', '#suffix') as $key) {
        $elements[$key] = isset($previous[$key]) ? $previous[$key] : NULL;
      }
    }
    // Render each of the children using drupal_render and concatenate them.
    if (!isset($content) || $content === '') {
      foreach ($children as $key) {
        $content .= _fideo_render($elements[$key]);
      }
    }
  }

  if (isset($content) && $content !== '') {
    $elements['#children'] = $content;
  }

  // Until now, we rendered the children, here we render the element itself
  if (!isset($elements['#printed'])) {
    $theme_func = !empty($elements['#renderer']) ? $elements['#renderer'] : (!empty($elements['#type']) ? 'fideo_'. $elements['#type'] : 'fideo_markup');

    $content = theme($theme_func, array('element' => $elements));
    $elements['#printed'] = TRUE;
  }

  if (isset($content) && $content !== '') {
    // Filter the outputted content and make any last changes before the
    // content is sent to the browser. The changes are made on $content
    // which allows the output'ed text to be filtered.
    if (isset($elements['#post_render'])) {
      foreach ($elements['#post_render'] as $function) {
        if (function_exists($function)) {
          $content = $function($content, $elements);
        }
      }
    }

    $prefix = isset($elements['#prefix']) ? $elements['#prefix'] : '';
    $suffix = isset($elements['#suffix']) ? $elements['#suffix'] : '';

    return $prefix . $content . $suffix;
  }
}

/**
 * Provides element information.
 *
 * @param string $type
 */
function _fideo_element_info($type) {
  static $cache;

  $basic_defaults = array(
    '#attributes' => array(),
    '#tree' => FALSE,
    '#parents' => array(),
  );

  if (!isset($cache)) {
    $cache = array();

    foreach (module_implements('fideo_elements') as $module) {
      $elements = module_invoke($module, 'fideo_elements');

      if (isset($elements) && is_array($elements)) {
        $cache = array_merge_recursive($cache, $elements);
      }

    }
    if (sizeof($cache)) {
      foreach ($cache as $element_type => $info) {
        $cache[$element_type] = array_merge_recursive($basic_defaults, $info);
      }
    }
  }

  return $cache[$type];
}

/**
 * Pre-render function for video tag.
 *
 * @param array $element
 *
 * @return array
 */
function _fideo_pre_render_video($element) {
  if (!empty($element['sources'])) {
    foreach (element_children($element['sources']) as $key) {
      $element['sources'][$key] += array(
        '#type' => 'source',
      );
    }
  }

  // Hackish fixes for MediaMosa that sometimes returns a streaming server
  // address to a video file. In that case, skip the source for HTML5. Also,
  // MediaMosa sometimes returns ticket URL's without file extension, making it
  // harder for certain browsers to recognize files when mimetype isn't enough.
  $sources = _fideo_element_by_type($element, 'source');
  foreach ($sources as $key) {
    if (!empty($element[$key]['#file'])) {
      $file = $element[$key]['#file'];
      // MediaMosa returns a streaming URL, even when 'plain' is supplied. In
      // that case, skip the item in normal rendering.
      if (!(drupal_substr($file, 0, 4) == 'http' && strpos($file, '://') !== FALSE && strpos($file, '://') <= 5)) {
        $element[$key]['#skip'] = TRUE;
        // No extension checking needed here anymore.
        continue;
      }

      // When no extension is found, fake an extension to help out some
      // browsers that have difficulty finding playable sources by mimetype.
      $extension = (strrpos($file, '.') !== FALSE ? drupal_substr($file, strrpos($file, '.') + 1) : '');
      if ((!$extension || drupal_strlen($extension) >= 4) && isset($element[$key]['#mimetype'])) {
        switch ($element[$key]['#mimetype']) {
          case 'video/mp4':
            $extension = 'mp4';
            break;
          case 'video/webm':
            $extension = 'webm';
            break;
          case 'video/ogv':
            $extension = 'ogg';
            break;
        }
        if ($extension) {
          $element[$key]['#file'] .= (strpos($element[$key]['#file'], '?') ? '&' : '?') . 'ext.' . $extension;
        }
      }
    }
  }

  if (!empty($element['tracks'])) {
    foreach (element_children($element['tracks']) as $key) {
      $element['tracks'][$key] += array(
        '#type' => 'track',
      );
    }
  }

  return $element;
}

/**
 * Helper function for finding an element based on type and a specific
 * attribute.
 *
 * @param array  $element
 * @param string $type
 * @param string $attr_name
 * @param string $attr_value
 *
 * @return array
 */
function _fideo_element_by_type($element, $type, $attr_name = NULL, $attr_value = NULL) {
  $keys = array();
  $is_array = is_array($type);
  foreach (element_children($element) as $key) {
    if ((($is_array && isset($element[$key]['#type']) && in_array($element[$key]['#type'], $type)) || (!$is_array && isset($element[$key]['#type']) && $element[$key]['#type'] == $type)) && (($attr_name && isset($element[$key][$attr_name]) && $element[$key][$attr_name] == $attr_value) || !$attr_name)) {
      $keys[] = $key;
    }
  }
  return $keys;
}

/**
 * Helper function for building the flashvars for the fallback player embed code.
 * @todo Move to fideo.render.inc
 */
function _fideo_fallback_flashvars($element, $source) {
  $config = array();
  $plugins = array();
  $playlist = array();

  // Build up the playlist.
  if (!empty($element['#poster'])) {
    // Add the poster as first item in the playlist array.
    // @todo Removed poster image because MediaMosa returns stills without
    //   extension and this is something Flowplayer can't handle.
    //$playlist[] = $element['#poster'];
  }

  // Main video item.
  $clip = array(
    'url' => $source['#file'],
    'autoBuffering' => TRUE,
    'autoPlay' => ($element['#autoplay'] ? TRUE : FALSE),
  );

  // Hack for RTMP streams, unfortunately needed because MediaMosa keeps
  // acting up.
  if (drupal_strtolower(drupal_substr($source['#file'], 0, 7)) == 'rtmp://') {
    // Explode on ampersand to separate file and server.
    // @todo This is super ugly, find a more stable way to do this.
    $parts = explode('&file=', $source['#file'], 2);

    $clip['provider'] = 'rtmp';
    $clip['url'] = 'mp4:' . $parts[1];
    $plugins['rtmp'] = array(
      'url' => 'flowplayer.rtmp-3.2.3.swf',
      'netConnectionUrl' => $parts[0],
    );
  }

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
        'body' => array('fontSize' => 18, 'fontFamily' => 'Arial,Helvetica,sans-serif', 'textAlign' => 'center', 'color' => '#ffffff'),
      ),
    );
  }

  $playlist[] = $clip;

  $config['playlist'] = $playlist;
  if (!empty($plugins)) {
    $config['plugins'] = $plugins;
  }

  $output = 'config='. drupal_json_encode($config);

  return $output;
}

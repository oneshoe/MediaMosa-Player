<?php
// $Id: fideo.demo.inc 287 2011-12-13 13:03:46Z thijs $

/**
 * @file
 * This file is for demo purpose only and show the format of the fideo.
 *
 * @author One Shoe
 */

/**
 * Demo structure for Drupal to Fideo conversion.
 */
function fideo_format() {
  $media = array(
    'primary' => array(
      '#type' => 'video',
      '#id' => 'fideo-primary',
      '#duration' => 1140,
      '#attributes' => array(
        'width' => 534,
        'height' => 400,
        'controls' => 'controls',
      ),
      '#poster' => 'http://labs.oneshoe.nl/fideo/assets/video/mozcamp_still.jpg',
      array(
        '#type' => 'source',
        '#mimetype' => 'video/mp4',
        '#codecs' => 'avc1.42E01E, mp4a.40.2',
        '#file' => 'http://labs.oneshoe.nl/fideo/assets/video/mozcamp.handbrake.mp4?1',
      ),
      'webm' => array(
        '#type' => 'source',
        '#mimetype' => 'video/webm',
        '#codecs' => 'vp8, vorbis',
        '#file' => 'http://labs.oneshoe.nl/fideo/assets/video/mozcamp.ffmpeg.webm?1',
      ),
      'ogv' => array(
        '#type' => 'source',
        '#mimetype' => 'video/ogg',
        '#codecs' => 'theora, vorbis',
        '#file' => 'http://labs.oneshoe.nl/fideo/assets/video/mozcamp.ffmpeg.ogv?1',
      ),
      array(
        '#type' => 'subtitles',
        '#lang' => 'nl',
        '#title' => t('Dutch'),
        '#file' => 'http://labs.oneshoe.nl/fideo/assets/video/mozcamp.captions.srt',
      ),
    ),
    'secondary' => array(
      '#type' => 'video',
      '#disabled' => TRUE,
      '#multistream' => 'fideo-primary',
      '#attributes' => array(
        'width' => 267, 'height' => 200, 'controls' => 'controls',
      ),
      '#poster' => 'http://labs.oneshoe.nl/fideo/assets/video/mozcamp_slide.jpg',
      array(
        '#type' => 'source',
        '#mimetype' => 'video/mp4',
        '#codecs' => 'avc1.42E01E, mp4a.40.2',
        '#file' => 'http://labs.oneshoe.nl/fideo/assets/video/mozcamp.handbrake.mp4?2',
      ),
      'webm' => array(
        '#type' => 'source',
        '#mimetype' => 'video/webm',
        '#codecs' => 'vp8, vorbis',
        '#file' => 'http://labs.oneshoe.nl/fideo/assets/video/mozcamp.ffmpeg.webm?2',
      ),
      'ogv' => array(
        '#type' => 'source',
        '#mimetype' => 'video/ogg',
        '#codecs' => 'theora, vorbis',
        '#file' => 'http://labs.oneshoe.nl/fideo/assets/video/mozcamp.ffmpeg.ogv?2',
      ),
    ),
    'slides' => array(
      '#type' => 'slides',
      '#target' => 'fideo-primary',
    ),
    'markers' => array(
      '#type' => 'markers',
      '#target' => 'fideo-primary',
    ),
  );

  $timecodes = array(2.031, 4.661, 8.75, 10.874, 12.945, 30.112, 73.847, 204.292, 483.464, 486.061, 487.62, 490.251, 492.863, 495.463, 498.076, 499.141, 500.688, 504.856, 508.48, 509.54, 512.214, 514.823, 515.875, 630.102, 639.983, 692.459);
  $alts = array(
    'MozCamp.odp - OpenOffice.org Impress',
    'MozCamp.odp - OpenOffice.org Impress',
    'MozCamp.odp - OpenOffice.org Impress
mozilla europe
"The Dutch Mozilla Community Exposed"
Wim Benes',
    'mozilla europe
"The Dutch Mozilla Community Exposed"
Wim Benes',
    'mozilla europe
"The Dutch Mozilla Community Exposed"
Wim Benes',
    'mozilla europe
"The Dutch Mozilla Community Exposed"
Wim Benes',
    'mozilla europe
Fryslan
Nederland
Europe
Planet',
    'mozilla europe
NETSCAPE.
NAVIGATOR',
    'MozCamp.odp - OpenOffice.org Impress
mozilla europe',
    'Live-Uitzending Auditorium SURFnet - Mozilla Firefox
Live
Uitzending Auditorium SURFnet',
    '',
    '',
    '',
    '',
    '',
    '',
    '',
    '',
    '',
    '',
    '',
    '',
    'mozilla europe
NETSCAPE.
NAVIGATOR',
    'mozilla
Krij Firefox 3 yn it Frysk',
    'mozilla messaging
Download Thunderbird 3 Beta 2',
    'mozilla europe
www.mozilla-nl.org
www.mozbrowser.nl
www.mozilla-europe.org',
  );

  for ($i = 0; $i < count($timecodes); $i++) {
    $media['slides'][$i] = array(
      '#type' => 'slide',
      '#start' => $timecodes[$i],
      '#file' => 'http://labs.oneshoe.nl/fideo/assets/stills/'. str_pad($i, 3, '0', STR_PAD_LEFT) . '.jpg',
      '#alt' => $alts[$i],
    );
  }

  $media['markers'][$i++] = array(
    '#type' => 'marker',
    '#start' => 3.3,
    '#end' => 6.4,
    '#value' => 'Introduction',
  );

  $media['markers'][$i++] = array(
    '#type' => 'marker',
    '#start' => 110.3,
    '#end' => 156.4,
    '#value' => 'Mozilla Firefox Frysian translation team',
  );
  $media['markers'][$i++] = array(
    '#type' => 'marker',
    '#start' => 1110.3,
    '#end' => 1100.4,
    '#value' => 'Time for questions',
  );
  $media['markers'][$i++] = array(
   '#type' => 'marker',
   '#start' => 400.3,
   '#end' => 410.4,
   '#value' => 'History: from Netscape, to Mozilla, to Firefox',
  );
  $media['markers'][$i++] = array(
    '#type' => 'marker',
    '#start' => 600.3,
    '#end' => 650.4,
    '#value' => '<img src="http://labs.oneshoe.nl/fideo/assets/stills/firefox_logo.png" width="100" height="100" alt="FireFox" /><br />The FireFox browser.',
  );

  // Create a wrapper.
  $media['#prefix'] = '<div id="fideo"><div class="fideo-wrapper">';
  $media['#suffix'] = '</div></div>';

  return $media;
}

/**
 * Menu callback function for demo purposes.
 *
 * @return rendered fideo player
 */
function fideo_testpage() {
  // Get the demo format from the function above.
  $media = fideo_format();

  // Render the video.
  $render = fideo_render($media);

  return $render;
}

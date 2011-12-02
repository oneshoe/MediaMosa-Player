<?php

/**
 * @file
 *
 * This file contains all the functions that will render the admin interface
 * in the back-end of Drupal.
 *
 * @author One Shoe
 */

/**
 * Renders the Fideo configuration form. The form values will get stored as
 * variables through Drupal's system_settings_form function.
 *
 * @return system_settings_form
 */
function fideo_configuration_form() {
  $form = array();

  $form['configuration'] = array(
    '#type' => 'fieldset',
    '#title' => t('Fideo configuration'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $form['configuration']['fideo_flowplayer_src'] = array(
    '#type' => 'textfield',
    '#title' => t('Flow player source path.'),
    '#description' => t("The path should be relative to Drupal's base path."),
    '#default_value' => variable_get('fideo_flowplayer_src', drupal_get_path('module', 'fideo') . FIDEO_DEFAULT_FLOWPLAYER_SRC),
    '#required' => TRUE,
  );

  $form['configuration']['fideo_qtip_src'] = array(
    '#type' => 'textfield',
    '#title' => t('qTip source path.'),
    '#description' => t("The path should be relative to Drupal's base path."),
    '#default_value' => variable_get('fideo_qtip_src', drupal_get_path('module', 'fideo') . FIDEO_DEFAULT_QTIP_SRC),
    '#required' => TRUE,
  );

  return system_settings_form($form);
}

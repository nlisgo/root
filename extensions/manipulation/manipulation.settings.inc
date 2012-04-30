<?php

/**
 * @file
 * Contains the theme settings form elements for the development extension.
 */

/**
 * Implements hook_extension_EXTENSION_theme_settings_form_alter().
 */
function root_extension_manipulation_theme_settings_form_alter($element, &$form, $form_state) {
  $element['root_css_exclude'] = array(
    '#type' => 'textarea',
    '#title' => t('Exclude CSS files'),
    '#description' => t("Enter one file per line. The '*' character is a wildcard. Each path is relative to a module, theme, profile, theme engine or the 'misc' folder."),
    // The paths are stored in an array.
    '#default_value' => implode("\n", (array) theme_get_setting('root_css_exclude')),
  );

  $element['root_js_exclude'] = array(
    '#type' => 'textarea',
    '#title' => t('Exclude JavaScript files'),
    '#description' => t("Enter one file per line. The '*' character is a wildcard. Each path is relative to a module, theme, profile, theme engine or the 'misc' folder."),
    // The paths are stored in an array.
    '#default_value' => implode("\n", (array) theme_get_setting('root_js_exclude')),
  );

  // We need a custom submit handler to store the CSS and JS paths as arrays.
  $form['#submit'][] = 'root_extension_manipulation_theme_settings_form_submit';

  return $element;
}

/**
 * Form submit handler for the manipulation theme settings extension.
 */
function root_extension_manipulation_theme_settings_form_submit($form, &$form_state) {
  $values = $form_state['values'];
  // Explode and trim the values for the CSS and JavaScript exclusion so we end
  // up with a clean array.
  foreach (array('css', 'js') as $type) {
    $exclude = array_filter(array_map('trim', explode("\n", $values['root_' . $type . '_exclude'])));
    form_set_value(array('#parents' => array('root_' . $type . '_exclude')), $exclude, $form_state);
  }
}

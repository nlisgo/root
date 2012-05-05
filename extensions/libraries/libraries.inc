<?php

/**
 * @file
 * Main extension file for the 'libraries' extension.
 */

/**
 * Implements hook_extension_EXTENSION_theme_settings_form_alter().
 */
function root_extension_libraries_theme_registry_alter(&$registry) {
  // If there are one or more libraries in the cache, add the preprocess hook.
  foreach (theme_get_setting('root_libraries') as $library => $settings) {
    if (!empty($settings['status'])) {
      $registry['html']['includes'][] = drupal_get_path('theme', 'root') . '/extensions/libraries/libraries.inc';
      $registry['html']['preprocess functions'][] = 'root_extension_libraries_preprocess_html';

      return;
    }
  }

}

/**
 * Implements hook_extension_EXTENSION_preproces_html().
 */
function root_extension_libraries_preprocess_html(&$variables) {
  foreach (theme_get_setting('root_libraries') as $library => $settings) {
    // Check if the library is enabled.
    if (!empty($settings['status'])) {
      $variant = !empty($settings['variant']) ? $settings['variant'] : NULL;
      dpm(libraries_load($library, $variant));
    }
  }
}

/**
 * Include callback for the CSS3PIE library.
 */
function root_library_pie_pre_load_callback($library, $version, $variant) {
  // Include the library depending on the inclusion method setting.
  if ($variant == 'js') {
    dpm('test');
  }
  else {
    $path = file_create_url('public://root/' . $GLOBALS['theme_key'] . '-pie-selectors.css');
    if (is_file($path)) {
      drupal_add_css($path, array('group' => CSS_THEME));
    }
  }
}

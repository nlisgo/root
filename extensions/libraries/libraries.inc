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
  if ($libraries = theme_get_setting('root_libraries')) {
    foreach ($libraries as $settings) {
      if (!empty($settings['status'])) {
        $registry['html']['includes'][] = drupal_get_path('theme', 'root') . '/extensions/libraries/libraries.inc';
        $registry['html']['preprocess functions'][] = 'root_extension_libraries_preprocess_html';

        return;
      }
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
      $response = libraries_load($library, $variant);

      if (isset($response['error message']) && user_access('administrate')) {
        drupal_set_message($response['error message'], 'error');
        if ($response['error'] == 'not found') {
          drupal_set_message(t('At least one of the enabled libraries has not been downloaded yet. Please make sure that all enabled libraries have been downloaded and placed in the correct folder. You can use the drush command "drush root-download-libraries" to do this automatically.'), 'warning', FALSE);
        }
      }
    }
  }
}

/**
 * Include callback for the CSS3PIE library.
 */
function root_library_pie_post_load_callback($library, $version, $variant) {
  // Include the selectors depending on the inclusion method setting.
  $settings = theme_get_setting('root_libraries');
  if (!empty($settings['css3pie']['selectors'])) {
    if ($variant == 'js') {
      $path = file_create_url('public://root/' . $GLOBALS['theme_key'] . '/pie-selectors.js');
      $path = substr($path, strlen($GLOBALS['base_url']) + 1);
      drupal_add_js($path, array('group' => JS_THEME, 'weight' => -10));
    }
    else {
      $path = file_create_url('public://root/' . $GLOBALS['theme_key'] . '/pie-selectors.css');
      $path = substr($path, strlen($GLOBALS['base_url']) + 1);
      drupal_add_css($path, array('group' => CSS_THEME, 'weight' => -10));
    }
  }
}

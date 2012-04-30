<?php

/**
 * @file
 * Main extension file for the 'libraries' extension.
 */

/**
 * Implements hook_extension_EXTENSION_theme_settings_form_alter().
 */
function root_extension_libraries_theme_registry_alter(&$registry) {
  $registry['html']['includes'][] = drupal_get_path('theme', 'root') . '/extensions/libraries/libraries.inc';
  $registry['html']['preprocess functions'][] = 'root_extension_libraries_preprocess_html';
}

/**
 * Implements hook_extension_EXTENSION_preproces_html().
 */
function root_extension_libraries_preprocess_html(&$variables) {
  // Iterate over all the existing libraries and include those that are active
  // in the current theme.
  foreach (root_invoke_all('root_libraries_info') as $library => $info) {
    if (theme_get_setting('root_library_' . $library)) {
      if (isset($info['include callback'])) {
        $info['include callback']($library, $info);
      }

      $attached = array(
        'js' => !empty($info['js']) ? $info['js'] : FALSE,
        'css' => !empty($info['css']) ? $info['css'] : FALSE,
        'library' => !empty($info['library']) ? $info['library'] : FALSE,
      );

      // Process all attached elements (libraries, javascript, styles) if there
      // are any.
      if ($attached = array_filter($attached)) {
        drupal_process_attached(array('#attached' => $attached));
      }
    }
  }
}

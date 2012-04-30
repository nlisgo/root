<?php

/**
 * @file
 * Main extension file for the 'compatibility' extension.
 */

/**
 * Implements hook_extension_EXTENSION_registry_alter().
 */
function root_extension_compatibility_theme_registry_alter(&$registry) {
  $registry['html']['includes'][] = drupal_get_path('theme', 'root') . '/extensions/compatibility/compatibility.inc';
  $registry['html']['preprocess functions'][] = 'root_extension_compatibility_preprocess_html';
}

/**
 * Implements hook_extension_EXTENSION_preproces_html().
 */
function root_extension_compatibility_preprocess_html(&$variables) {
  // Add a meta tag for the viewport specifications.
  if (theme_get_setting('root_viewport')) {
    $initial = theme_get_setting('root_viewport_initial_scale');
    $min = theme_get_setting('root_viewport_min_scale');
    $max = theme_get_setting('root_viewport_max_scale');
    $scalable = theme_get_setting('root_viewport_user_scalable') ? 'yes' : 'no';

    $meta = array(
      '#tag' => 'meta',
      '#attributes' => array(
        'name' => 'viewport',
        'content' => 'width=device-width, initial-scale=' . $initial . ', maximum-scale=' . $max . ', minimum-scale=' . $min . ', user-scalable=' . $scalable,
      ),
    );

    drupal_add_html_head($meta, 'root-viewport');
  }

  // Add a meta tag to enforce "edge" (highest available) rendering mode for
  // Internet Explorer.
  if (theme_get_setting('root_chrome_edge')) {
    $meta = array(
      '#tag' => 'meta',
      '#attributes' => array(
        'http-equiv' => 'X-UA-Compatible',
        'content' => 'IE=edge, chrome=1',
      ),
    );

    drupal_add_html_head($meta, 'root-chrome-edge');
  }
}

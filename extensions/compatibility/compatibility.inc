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

  // Send X-UA-Compatible HTTP header to force IE to use the most recent
  // rendering engine or use Chrome's frame rendering engine if available.
  // This also prevents the IE compatibility mode button to appear when using
  // conditional classes on the html tag.
  if (theme_get_setting('root_chrome_edge') && drupal_get_http_header('X-UA-Compatible') === NULL) {
    drupal_add_http_header('X-UA-Compatible', 'IE=edge,chrome=1');
  }
}

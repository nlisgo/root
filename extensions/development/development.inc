<?php

/**
 * @file
 * Main extension file for the 'development' extension.
 */

/**
 * Implements hook_extension_EXTENSION_registry_alter().
 */
function root_extension_development_theme_registry_alter(&$registry) {
  if (theme_get_setting('root_viewport_widget')) {
    $registry['html']['includes'][] = drupal_get_path('theme', 'root') . '/extensions/development/development.inc';
    $registry['html']['preprocess functions'][] = 'root_extension_development_preprocess_html';
  }
}

/**
 * Implements hook_extension_EXTENSION_preproces_html().
 */
function root_extension_development_preprocess_html(&$variables) {

}

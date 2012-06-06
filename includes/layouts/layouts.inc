<?php

/**
 * @file
 * Main extension file for the 'development' extension.
 */

/**
 * Implements hook_extension_EXTENSION_registry_alter().
 */
function omega_extension_layouts_theme_registry_alter(&$registry) {
  $registry['page']['includes'][] = drupal_get_path('theme', 'omega') . '/includes/layouts/layouts.inc';
  $registry['page']['process functions'][] = 'omega_extension_layouts_process_page';

  // Add a separate pre-process function just for loading the layout from the
  // theme settings. This way subthemes can change the layout before it gets
  // turned into a theme hook suggestion.
  $registry['page']['preprocess functions'][] = 'omega_extension_layouts_preprocess_page';

  // Append the information for all available layouts to the theme registry so
  // we don't need to load it from scratch later on.
  $registry['page']['layouts'] = omega_layouts_info();

  // We don't need to cache unsupported layouts.
  foreach ($registry['page']['layouts'] as $layout => $info) {
    if (!$info['supported']) {
      unset($registry['page']['layouts'][$layout]);
      break;
    }

    foreach (array('css', 'js') as $type) {
      if (!empty($info['attached'][$type])) {
        foreach ($registry['page']['layouts'][$layout]['attached'][$type] as $file => $options) {
          unset($registry['page']['layouts'][$layout]['attached'][$type][$file]);

          if ($file = omega_theme_trail_find_file($file)) {
            $registry['page']['layouts'][$layout]['attached'][$type][$file] = $options;
          }
        }
      }
    }
  }
}

/**
 * Implements hook_extension_EXTENSION_preprocess_page().
 */
function omega_extension_layouts_preprocess_page(&$variables) {
  $variables['layout'] = theme_get_setting('omega_layout');
}

/**
 * Implements hook_extension_EXTENSION_process_page().
 */
function omega_extension_layouts_process_page(&$variables) {
  if (!empty($variables['layout'])) {
    $registry = theme_get_registry();

    // Check if the provided layout is valid.
    if (isset($registry['page']['layouts'][$variables['layout']]) && $registry['page']['layouts'][$variables['layout']]['supported']) {
      $layout = $registry['page']['layouts'][$variables['layout']];
      $suggestion = 'page__' . str_replace('-', '_', $layout['template']);

      foreach ($variables['theme_hook_suggestions'] as &$item)  {
        if (strpos($suggestion, 'page__') === 0) {
          $item = $suggestion . '__' . substr($suggestion, 6);
        }
      }

      // Prepend the base theme hook suggestions.
      array_unshift($variables['theme_hook_suggestions'], $suggestion);

      // Load all the assets for the current layout.
      drupal_process_attached(array('#attached' => $layout['attached']));
    }
  }
}

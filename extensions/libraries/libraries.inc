<?php

/**
 * @file
 * Main extension file for the 'libraries' extension.
 */

/**
 * Implements hook_extension_EXTENSION_theme_settings_form_alter().
 */
function root_extension_libraries_theme_registry_alter(&$registry) {
  foreach (root_invoke_all('root_libraries_info') as $library => $info) {
    // Check if the library is enabled.
    if (theme_get_setting('root_library_' . $library)) {
      // Cache this library in the theme registry. This way we don't have to
      // invoke the 'root_libraries_info' hook on every page.
      $registry['html']['root libraries'][$library] = $info;
    }
  }

  // If there are one or more libraries in the cache, add the preprocess hook.
  if (!empty($registry['html']['root libraries'])) {
    $registry['html']['includes'][] = drupal_get_path('theme', 'root') . '/extensions/libraries/libraries.inc';
    $registry['html']['preprocess functions'][] = 'root_extension_libraries_preprocess_html';
  }
}

/**
 * Implements hook_extension_EXTENSION_preproces_html().
 */
function root_extension_libraries_preprocess_html(&$variables) {
  $registry = theme_get_registry();

  // Load the production version (minified) of the library if we are in
  // production mode and the library actually has a minified version.
  // This only affects JavaScript files.
  $production = theme_get_setting('root_minified_libraries');

  // This hook should only get invoked if the 'class rules' property has at
  // least one rule in it. This check is just for good measure.
  if (!empty($registry['html']['root libraries'])) {
    foreach ($registry['html']['root libraries'] as $library => $info) {
      $callback = isset($info['include callback']) ? $info['include callback'] : 'root_library_default_include_callback';

      // Include the file that the callback resides in (if defined).
      if (isset($info['include callback file'])) {
        require_once $info['include callback file'];
      }

      $callback($library, $info, $GLOBALS['theme_key'], $production);
    }
  }
}

/**
 * Default include callback for libraries.
 *
 * @param $library
 *   The machine-readable name of a library.
 * @param $info
 *   The info array of a library.
 * @param $theme
 *   The machine-readable name of the theme to load the library for.
 * @param $production
 *   (Optional) Whether to load the production version of the library (if any).
 *   Defaults to FALSE.
 */
function root_library_default_include_callback($library, $info, $theme, $production = FALSE) {
  // If the 'path' is set in the library definition use that.
  if (isset($info['path'])) {
    $path = $info['path'];
  }
  // Otherwise retrieve the path using the libraries module or use the theme
  // directory as a fallback.
  else {
    $path = module_exists('libraries') ? libraries_get_path($library) : drupal_get_path('theme', $theme) . '/libraries/' . $library;
  }

  // Load the library by iterating over all the attached files and invoking the
  // proper callback for each of them.
  foreach ($info['library'] as $type => $files) {
    foreach ($files as $file => $options) {
      switch ($type) {
        case 'css':
        case 'js':
          if ($type == 'js' && $production == TRUE) {
            // Replace the default version of the file with the minified version
            // if one exists.
            $minified = str_replace('.js', '.min.js', $file);
            if (is_file($path . '/' . $minified)) {
              $file = $minified;
            }
          }

          // Use the well-known drupal_add_js() and drupal_add_css() functions
          // to load the attached file with the defined options.
          $function = 'drupal_add_' . $type;
          $function($path . '/' . $file, $options);
          break;

        default:
          // If the type is not 'js' and not 'css' it is considered a function
          // name itself.
          if (function_exists($type)) {
            $type($file, $path, $options, $library, $info, $theme);
          }
          break;
      }
    }
  }
}

/**
 * Include callback for the CSS3PIE library.
 */
function root_pie_include($library, $info, $theme) {
  $file = theme_get_setting('root_library_pie_inclusion', $theme);

  // Include the library depending on the inclusion method setting.
  switch ($file) {
    case 'pie.js':
      $path = module_exists('libraries') ? libraries_get_path('pie') : drupal_get_path('theme', $theme) . '/libraries/pie';
      drupal_add_js($path . '/' . $file, array('group' => JS_THEME, 'browsers' => array()));
      break;

    case 'pie.htc':
    case 'pie.php':
    default:
      $path = file_create_url('public://root/' . $theme . '-pie-selectors.css');
      if (is_file($path)) {
        drupal_add_css($path, array('group' => CSS_THEME));
        break;
      }
  }
}

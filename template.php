<?php

/**
 * @file
 * Template overrides and (pre-)process hooks for the Root base theme.
 */

require_once dirname(__FILE__) . '/includes/root.inc';

/**
 * Slightly hacky performance tweak for theme_get_setting(). This resides
 * outside of any function declaration to make sure that it runs directly after
 * the theme has been initialized.
 *
 * Instead of rebuilding the theme settings array on every page load we are
 * caching the content of the static cache in the database after it has been
 * built initially. This is quite a bit faster than running all the code in
 * theme_get_setting() on every page.
 *
 * By checking whether the global 'theme' and 'theme_key' properties are
 * identical we make sure that we don't interfere with any of the theme settings
 * pages and only use this feature when actually rendering a page with this
 * theme.
 *
 * @see theme_get_setting()
 */
if ($GLOBALS['theme'] == $GLOBALS['theme_key'] && !$static = &drupal_static('theme_get_setting')) {
  if ($cache = cache_get('theme_settings:' . $GLOBALS['theme'])) {
    // If the cache entry exists, populate the static theme settings array with
    // its data. This prevents the theme settings from being rebuilt on every
    // page load.
    $static[$GLOBALS['theme']] = $cache->data;
  }
  else {
    // Invoke theme_get_setting() with a random argument to build the theme
    // settings array and populate the static cache.
    theme_get_setting('foo');
    // Extract the theme settings from the previously populated static cache.
    $static = &drupal_static('theme_get_setting');
    // Cache the theme settings in the database.
    cache_set('theme_settings:' . $GLOBALS['theme'], $static[$GLOBALS['theme']]);
  }
}

/**
 * Rebuild the theme registry on every page load if the development extension
 * is enabled and configured to do so. This also lives outside of any function
 * declaration to make sure that the registry is rebuilt before invoking any
 * theme hooks.
 */
if (root_extension_is_enabled('development') && theme_get_setting('root_rebuild_theme_registry')) {
  drupal_theme_rebuild();

  if (flood_is_allowed('root_rebuild_registry_warning', 1)) {
    // Alert the user that the theme registry is being rebuilt on every request.
    flood_register_event('root_rebuild_registry_warning');
    drupal_set_message(t('The theme registry is being rebuilt on every request. Remember to <a href="!url">turn off</a> this feature on production websites.', array("!url" => url('admin/appearance/settings/' . $GLOBALS['theme']))));
  }
}

/**
 * Implements hook_element_info_alter().
 */
function root_element_info_alter(&$elements) {
  if (theme_get_setting('root_media_queries_inline') && variable_get('preprocess_css', FALSE) && (!defined('MAINTENANCE_MODE') || MAINTENANCE_MODE != 'update')) {
    array_unshift($elements['styles']['#pre_render'], 'root_css_preprocessor');
  }
}

/**
 * Implements hook_css_alter().
 */
function root_css_alter(&$css) {
  if (root_extension_is_enabled('manipulation') && $exclude = theme_get_setting('root_css_exclude')) {
    root_exclude_assets($css, $exclude);
  }

  // The CSS_SYSTEM aggregation group doesn't make any sense. Therefore, we are
  // pre-pending it to the CSS_DEFAULT group. This has the same effect as giving
  // it a separate (low-weighted) group but also allows it to be aggregated
  // together with the rest of the CSS.
  foreach ($css as $key => $item) {
    if ($item['group'] == CSS_SYSTEM) {
      $css[$key]['group'] = CSS_DEFAULT;
      $css[$key]['weight'] = $item['weight'] - 100;
    }
  }
}

/**
 * Implements hook_js_alter().
 */
function root_js_alter(&$js) {
  if (root_extension_is_enabled('manipulation') && $exclude = theme_get_setting('root_js_exclude')) {
    root_exclude_assets($js, $exclude);
  }
}

/**
 * Implements hook_theme_registry_alter().
 *
 * Allows subthemes to split preprocess / process / theme code across separate
 * files to keep the main template.php file clean. This is really fast because
 * it uses the theme registry to cache the pathes to the files that it finds.
 */
function root_theme_registry_alter(&$registry) {
  // Load the theme trail and all enabled extensions.
  $trail = root_theme_trail();
  $extensions = root_extensions();

  foreach ($trail as $key => $theme) {
    foreach (array('preprocess', 'process', 'theme') as $type) {
      $path = drupal_get_path('theme', $key);
      // Only look for files that match the 'something.preprocess.inc' pattern.
      $mask = '/.' . $type . '.inc$/';
      // This is the length of the suffix (e.g. '.preprocess') of the basename
      // of a file.
      $strlen = -(strlen($type) + 1);

      // Recursively scan the folder for the current step for (pre-)process
      // files and write them to the registry.
      foreach (file_scan_directory($path . '/' . $type, $mask) as $item) {
        $hook = substr($item->name, 0, $strlen);

        if (array_key_exists($hook, $registry)) {
          // By adding this file to the 'includes' array we make sure that it is
          // available when the hook is executed.
          $registry[$hook]['includes'][] = $item->uri;

          if ($type == 'theme') {
            // Remove the template file reference as this is now handled by a
            // theme function.
            unset($registry[$hook]['template']);
            $registry[$hook]['type'] = 'theme_engine';
            $registry[$hook]['theme path'] = $path;
            $registry[$hook]['function'] = $key . '_' . $hook;
          }
          else {
            // Append the included preprocess hook to the array of functions.
            $registry[$hook][$type . ' functions'][] = $key . '_' . $type . '_' . $hook;
          }
        }
      }
    }
  }

  // Include the main extension file for every enabled extensions. This is
  // required for the next step (allowing extensions to register hooks in the
  // theme registry).
  foreach ($extensions as $extension) {
    root_theme_trail_load_include('inc', 'extensions/' . $extension . '/' . $extension);

    // Give every enabled extension a chance to alter the theme registry.
    foreach ($trail as $key => $theme) {
      $hook = $key . '_extension_' . $extension . '_theme_registry_alter';
      if (function_exists($hook)) {
        $hook($registry);
      }
    }
  }
}

/**
 * Implements hook_html_head_alter().
 */
function root_html_head_alter(&$head) {
  // Simplify the meta tag for character encoding.
  $head['system_meta_content_type']['#attributes'] = array('charset' => str_replace('text/html; charset=', '', $head['system_meta_content_type']['#attributes']['content']));
}

/**
 * Implements hook_root_libraries_info().
 */
function root_root_libraries_info() {
  // Use the libraries module to identify the path of the library if it is
  // enabled. Otherwise just default to sites/all/libraries.
  $path = module_exists('libraries') ? libraries_get_path('selectivizr') : 'sites/all/libraries/selectivizr/selectivizr.js';
  $libraries['selectivizr'] = array(
    'label' => t('Selectivizr'),
    'description' => t('Selectivizr is a JavaScript utility that emulates CSS3 pseudo-classes and attribute selectors in Internet Explorer 6-8. Simply include the script in your pages and selectivizr will do the rest.'),
    'author' => 'Keith Clark',
    'website' => 'http://selectivizr.com/',
    'js' => array($path),
  );

  $libraries['pie'] = array(
    'label' => t('CSS3 PIE'),
    'description' => t('PIE makes Internet Explorer 6-9 capable of rendering several of the most useful CSS3 decoration features.'),
    'author' => 'Keith Clark',
    'website' => 'http://css3pie.com/',
    'include callback' => 'root_pie_include',
  );

  return $libraries;
}

/**
 * Library options form callback.
 */
function root_library_options_form($element, &$form, $form_state, $library, $info) {
  // Some of our libraries provide additional options.
  switch ($library) {
    case 'pie':
      $element['root_library_pie_inclusion'] = array(
        '#title' => t('Inclusion method'),
        '#type' => 'select',
        '#options' => array('pie.htc' => t('HTML Component (default)'), 'pie.php' => t('PHP Script'), 'pie.js' => ('JavaScript File (not recommended)')),
        '#description' => t('Please refer to the <a href="!link">official documentation</a> to learn about the advantages and disadvantages of the different inclusion methods.', array('!link' => 'http://css3pie.com/documentation/')),
      );

      // Pull the theme key from the form arguments.
      $theme = $form_state['build_info']['args'][0];
      // Load the contents of the current version of the PIE selector css file.
      $source = 'public://root/' . $theme . '-pie-selectors.css';

      // Load the current PIE selectors file if one exists.
      if ($selectors = @file_get_contents($source)) {
        // We need to sanitize the output for our textarea.
        $selectors = preg_replace('/\{ behavior: url\((.*)\); \}/', '', $selectors);
        $selectors = array_filter(array_map('trim', explode(',', $selectors)));
        $selectors = implode("\n", $selectors);
      }

      $element['root_library_pie_selectors'] = array(
        '#title' => t('Selectors'),
        '#type' => 'textarea',
        '#description' => t("You can use this textarea to define all the CSS rules that you want to apply the PIE behavior to. Define one CSS selector per line. Note: The value of this field is not stored as a theme settings as it is directly written to a .css file in the <a href=\"!url\">public file system</a> to not clutter the theme settings array. Therefore, it won't get exported if you choose to export your theme settings.", array('!url' => file_create_url($source))),
        '#default_value' => $selectors ? $selectors : '',
        '#states' => array(
          'invisible' => array(
            ':input[name="root_library_pie_inclusion"]' => array('value' => 'pie.js'),
          ),
        ),
      );

      // We need to provide a submit handler to create a CSS file for the
      // defined selectors and remove them from the theme settings array.
      $form['#submit'][] = 'root_library_pie_selectors_submit';
      break;
  }

  return $element;
}

/**
 * Include callback for the CSS3PIE library.
 */
function root_pie_include($library, $info) {
  $file = theme_get_setting('root_library_pie_inclusion');

  // Include the library depending on the inclusion method setting.
  switch ($file) {
    case 'pie.js':
      $path = drupal_get_path('theme', $GLOBALS['theme_key']) . '/libraries/pie/' . $file;
      drupal_add_js($path, array('group' => JS_THEME, 'browsers' => array()));
      break;

    case 'pie.htc':
    case 'pie.php':
    default:
      $path = file_create_url('public://root/' . $GLOBALS['theme_key'] . '-pie-selectors.css');
      if (is_file($path)) {
        drupal_add_css($path, array('group' => CSS_THEME));
        break;
      }
  }
}

<?php

/**
 * @file
 * Helper functions for the Root base theme.
 */

/**
 * Builds the full theme trail (deepest base theme first, subtheme last) for a
 * theme.
 *
 * @param $theme
 *   (Optional) The key (machine-readable name) of a theme. Defaults to the key
 *   of the current theme.
 *
 * @return array
 *   An array of all themes in the trail, keyed by theme key.
 */
function root_theme_trail($theme = NULL) {
  $theme = isset($theme) ? $theme : $GLOBALS['theme_key'];
  $static = &drupal_static(__FUNCTION__);

  if (!isset($static[$theme])) {
    $static[$theme] = array();
    $themes = list_themes();

    if (isset($themes[$theme]->info['base theme'])) {
      foreach (system_find_base_themes($themes, $theme) as $base => $name) {
        if ($name && isset($themes[$base])) {
          $static[$theme][$base] = $themes[$base]->info['name'];
        }
      }
    }

    // Add our current subtheme ($key) to that array.
    $static[$theme][$theme] = $themes[$theme]->info['name'];
  }

  return $static[$theme];
}

/**
 * Pre-processes CSS files so that CSS files that have 'preprocess_media' set to
 * TRUE are set to media="all" while having their former media query added to
 * the file content.
 *
 * @param $elements
 *   An array of CSS files as in drupal_pre_render_styles().
 *
 * @return array
 *   An array of preprocessed CSS files.
 *
 * @see drupal_pre_render_styles()
 */
function root_css_preprocessor($elements) {
  foreach ($elements['#items'] as &$item) {
    if ($item['type'] == 'file' && $item['preprocess'] && $item['media'] != 'all') {
      $item['data'] = root_css_cache_media_queries($item);
      $item['media'] = 'all';
    }
  }
  return $elements;
}

/**
 * Optimizes CSS aggregation by creating a cached version of each CSS file that,
 * instead of using the 'media' attribute on the styles tag, writes the media
 * query into the file itself using the '@media { ... }' syntax.
 *
 * This prevents unnecessary sprouting of new CSS aggregation.
 *
 * @see drupal_build_css_cache().
 */
function root_css_cache_media_queries($item) {
  $map = variable_get('drupal_css_cache_files', array());
  $key = hash('sha256', serialize($item));
  $uri = isset($map[$key]) ? $map[$key] : NULL;

  if (empty($uri) || !file_exists($uri)) {
    // Build the base URL of this CSS file: start with the full URL.
    $base = file_create_url($item['data']);
    $base = substr($base, 0, strrpos($base, '/'));

    if (substr($base, 0, strlen($GLOBALS['base_root'])) == $GLOBALS['base_root']) {
      $base = substr($base, strlen($GLOBALS['base_root']));
    }

    _drupal_build_css_path(NULL, $base . '/');

    $data = drupal_load_stylesheet($item['data'], TRUE);

    // Anchor all paths in the CSS with its base URL, ignoring external and absolute paths.
    $data = preg_replace_callback('/url\(\s*[\'"]?(?![a-z]+:|\/+)([^\'")]+)[\'"]?\s*\)/i', '_drupal_build_css_path', $data);
    $data = '@media ' . $item['media'] . '{' . $data . '}';

    // Create the css/ within the files folder.
    $directory = 'public://css';
    $uri = $directory . '/css_' . drupal_hash_base64($data) . '.css';

    // Create the CSS file.
    file_prepare_directory($directory, FILE_CREATE_DIRECTORY);
    if (!file_exists($uri) && !file_unmanaged_save_data($data, $uri, FILE_EXISTS_REPLACE)) {
      return FALSE;
    }

    // If CSS gzip compression is enabled, clean URLs are enabled (which means
    // that rewrite rules are working) and the zlib extension is available then
    // create a gzipped version of this file. This file is served conditionally
    // to browsers that accept gzip using .htaccess rules.
    if (variable_get('css_gzip_compression', TRUE) && variable_get('clean_url', 0) && extension_loaded('zlib')) {
      if (!file_exists($uri . '.gz') && !file_unmanaged_save_data(gzencode($data, 9, FORCE_GZIP), $uri . '.gz', FILE_EXISTS_REPLACE)) {
        return FALSE;
      }
    }

    // Save the updated map.
    $map[$key] = $uri;

    // Write the updated map into the variable.
    variable_set('drupal_css_cache_files', $map);
  }

  return $uri;
}

/**
 * Helper function for eliminating elements from an array using a simplified
 * regex pattern.
 *
 * @param $elements
 *   The array of elements that should have some elements nuked.
 * @param $exclude
 *   An array of strings that should be matched against the keys of the array
 *   of elements.
 *
 * @return array
 *   The purged array.
 */
function root_exclude_assets(&$elements, $exclude) {
  // For optimization reasons we load the theme trail to check whether a
  // namespace matches the machine-readable name of one of the themes in the
  // trail.
  $trail = root_theme_trail();

  foreach ($exclude as $item) {
    $path = '';
    // The first segment (everything before the first slash) is the namespace.
    list($namespace) = explode('/', $item);

    // Check if the namespace refers to a file residing in the 'misc' folder.
    if ($namespace == 'misc') {
      $path = DRUPAL_ROOT . '/misc';
    }
    // Check if the namespace refers to a theme.
    elseif (array_key_exists($namespace, $trail)) {
      $path = drupal_get_path('theme', $namespace);
    }
    else {
      // Otherwise, check if it refers to a module, profile or theme engine.
      foreach (array('module', 'profile', 'theme_engine') as $type) {
        if ($path = drupal_get_path($type, $namespace)) {
          break;
        }
      }
    }

    // If a namespace could be identified, use its path as a prefix, otherwise
    // use the plain file path as provided.
    $item = $path ? $path . '/' . substr($item, strlen($namespace) + 1) : $item;
    $item = preg_quote($item, '/');
    // Turn the * wildcards into actual regex wildcards and make sure that, if
    // a .css file is targeted directly we are also removing the RTL version of
    // that file.
    $item = str_replace(array('\*', '\.css'), array('(.*)', '(\.css|-rtl\.css)'), $item);

    // Look up all elements that match this exclusion pattern.
    $filtered = preg_grep("/^$item$/", array_keys($elements));
    $elements = array_diff_key($elements, array_flip($filtered));
  }
}

/**
 * Retrieves the array of enabled extensions for a theme. Every feature that has
 * been toggled on in the themes .info file, excluding the default theme
 * features (as defined in core), is considered an extension. We need this
 * information to make it easy for us to conditionally build the theme settings
 * form based on which extensions are enabled. Each extension can define a
 * theme settings form altering function named
 * 'THEMENAME_extension_EXTENSION_theme_settings_form_alter()' and optionally
 * put it into 'THEME_ROOT/includes/extensions/EXTENSION.settings.inc' to have
 * it automatically included whenever the theme settings form is displayed
 * (given that the extension is currently enabled). Each extension can also
 * define a 'THEMENAME_extension_EXTENSION_theme_registry_alter()' function and
 * optionally put it into 'THEME_ROOT/includes/extensions/EXTENSION.inc' to
 * register custom hooks with the theme registry.
 *
 * @param $theme
 *   (Optional) The key (machine-readable name) of a theme. Defaults to the key
 *   of the current theme.
 *
 * @return array
 *   The theme info array of the passed or current theme.
 *
 * @see _system_default_theme_features()
 * @see root_extension_development_theme_settings_form_alter()
 * @see root_extension_development_theme_registry_alter()
 */
function root_extensions($theme = NULL) {
  $features = &drupal_static(__FUNCTION__);
  $theme = isset($theme) ? $theme : $GLOBALS['theme_key'];

  if (!isset($features[$theme])) {
    $features[$theme] = array();
    foreach (array_reverse(root_theme_trail($theme)) as $key => $title) {
      $info = root_theme_info($key);
      if (!empty($info['features'])) {
        // The default theme features are not considered 'extensions' per-se.
        $features[$theme] = array_diff($info['features'], _system_default_theme_features());
        break;
      }
    }
  }

  return $features[$theme];
}

/**
 * Checks whether an extension is enabled.
 *
 * Core maintains the state of it's features by caching 'toggle' flags in the
 * theme settings array. This only works for the default features provided by
 * core and is a little bit wonky as those are not really theme settings. Hence,
 * we need a custom function for checking whether an extension is enabled.
 *
 * @param $extension
 *   The name of an extension.
 * @param $theme
 *   (Optional) The key (machine-readable name) of a theme. Defaults to the key
 *   of the current theme.
 *
 * @return bool
 *   TRUE if the extension is enabled, FALSE otherwise.
 *
 * @see theme_get_setting()
 * @see root_extensions()
 */
function root_extension_is_enabled($extension, $theme = NULL) {
  return in_array($extension, root_extensions($theme));
}

/**
 * Retrieves the full info array of a theme.
 *
 * @param $theme
 *   (Optional) The key (machine-readable name) of a theme. Defaults to the key
 *   of the current theme.
 *
 * @return array
 *   The theme info array of the passed or current theme.
 */
function root_theme_info($theme = NULL) {
  $theme = isset($theme) ? $theme : $GLOBALS['theme_key'];

  // If this is the current theme, just load the theme info from the globals.
  // Note: The global 'theme_key' property is not reliable in this case because
  // it gets overridden on theme settings pages.
  if ($theme == $GLOBALS['theme']) {
    return $GLOBALS['theme_info']->info;
  }

  $themes = list_themes();
  return $themes[$theme]->info;
}

/**
 * Includes all occurrences of a file in all themes in the theme trail. This is
 * very similar to module_load_include().
 *
 * @param $type
 *   The include file's type (file extension).
 * @param $name
 *   The base file name (without the $type extension).
 * @param $theme
 *   (Optional) The key (machine-readable name) of a theme. Defaults to the key
 *   of the current theme.
 *
 * @see module_load_include()
 */
function root_theme_trail_load_include($type, $name, $theme = NULL) {
  $theme = isset($theme) ? $theme : $GLOBALS['theme_key'];

  foreach (root_theme_trail($theme) as $key => $title) {
    // Check if the requested file exists in any of the themes in the theme
    // trail and include all occurrences.
    $file = DRUPAL_ROOT . '/' . drupal_get_path('theme', $key) . "/$name.$type";
    if (is_file($file)) {
      include_once $file;
    }
  }
}

/**
 * Invoke a hook in all themes in the theme trail that implement it.
 *
 * @param $hook
 *   The name of the hook to invoke.
 * @param $theme
 *   (Optional) The key (machine-readable name) of a theme. Defaults to the key
 *   of the current theme.
 * @param ...
 *   Arguments to pass to the hook.
 *
 * @return array
 *   An array of return values of the hook implementations. If themes return
 *   arrays from their implementations, those are merged into one array.
 *
 * @see module_invoke_all()
 */
function root_invoke_all($hook, $theme = NULL) {
  $args = func_get_args();
  // Remove $hook and $theme from the arguments.
  unset($args[0], $args[1]);
  $return = array();
  foreach (root_theme_trail($theme) as $key => $name) {
    $function = $key . '_' . $hook;
    if (function_exists($function)) {
      $result = call_user_func_array($function, $args);
      if (isset($result) && is_array($result)) {
        // Append the 'theme' property to each array element.
        foreach ($result as &$item) {
          $item['theme'] = $key;
        }
        $return = array_merge_recursive($return, $result);
      }
      elseif (isset($result)) {
        $return[] = $result;
      }
    }
  }
  return $return;
}

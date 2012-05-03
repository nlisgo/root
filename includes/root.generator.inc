<?php

/**
 * @file
 * Provides functions for generating subthemes of the Root base theme.
 */

/**
 * Retrieves all themes that can be used as starterkits for a given base theme.
 *
 * @param $base
 *   (Optional) The machine-readable name of a base theme to filter all the
 *   available starterkits for.
 *
 * @return array
 *   An array of theme names keyed with the machine-readable name of each theme.
 */
function root_generator_starterkits($base = NULL) {
  $options = array();

  // Retrieve all the possible starterkits.
  foreach (list_themes() as $name => $theme) {
    $info = $theme->info;

    // A starterkit is always flagged as such in the .info file.
    if (!empty($info['starterkit']) && !empty($info['base theme']) && (!isset($base) || $info['base theme'] == $base)) {
      $options[$name] = $info['name'] . (!empty($info['description']) ? ': ' . $info['description'] : '');
    }
  }

  return $options;
}

/**
 * Recursively copies a directory from a given source to a given destination.
 *
 * @param $source
 *   The source path.
 * @param $destination
 *   The destination path.
 *
 * @return bool
 *   TRUE if the operation succeeded, FALSE otherwise.
 */
function root_generator_copy_recursive($source, $destination) {
  if (is_dir($source)) {
    if (!file_prepare_directory($destination, FILE_CREATE_DIRECTORY)) {
      watchdog('root', t('There was an error while trying to prepare a directory in @destination for copying over @source.', array('@destination' => $destination, '@source' => $source)));
      return FALSE;
    }
    $directory = dir($source);

    while (FALSE !== ($read = $directory->read())) {
      if ($read != '.' && $read != '..' ) {
        if (!root_generator_copy_recursive($source . '/' . $read, $destination . '/' . $read)) {
          return FALSE;
        }
      }
    }
    $directory->close();
  }
  else {
    file_unmanaged_copy($source, $destination);
  }

  return TRUE;
}

/**
 * Moves a directory from a given source to a given destination.
 *
 * @param $source
 *   The source path.
 * @param $destination
 *   The destination path.
 *
 * @return bool
 *   TRUE if the operation succeeded, FALSE otherwise.
 *
 * @see root_generator_copy_recursive()
 */
function root_generator_move($source, $destination) {
  if (root_generator_copy_recursive($source, $destination)) {
    // Delete the files in the source path after they have been successfullly
    // copied to their destination.
    file_unmanaged_delete_recursive($source);

    return TRUE;
  }

  return FALSE;
}

/**
 * Recursively rewrites (and renames) all files in a given path.
 *
 * @param $path
 *   The path to rewrite all files in.
 * @param $search
 *   The string(s) to look for when replacing the file names and contents. Can
 *   be an array or a string.
 * @param $replace
 *   The string(s) to replace $search with. Can be an array or a string.
 *
 * @return bool
 *   TRUE if the operation succeeded, FALSE otherwise.
 *
 * @see root_generator_replace_contents()
 * @see str_replace()
 */
function root_generator_rewrite_recursive($path, $search, $replace) {
  if ($path !== ($new = str_replace($search, $replace, $path))) {
    // First, try to rename (move) the file if the name was changed.
    if (!$path = file_unmanaged_move($path, $new, FILE_EXISTS_REPLACE)) {
      return FALSE;
    };
  }

  if (is_dir($path)) {
    // If the file actually is a directory, proceed with the recursion.
    $directory = dir($path);

    while (FALSE !== ($read = $directory->read())) {
      if ($read != '.' && $read != '..' ) {
        if (!root_generator_rewrite_recursive($path . '/' . $read, $search, $replace)) {
          return FALSE;
        }
      }
    }

    $directory->close();
  }
  elseif (is_file($path)) {
    // If it is a file, try to replace its the contents.
    if (!root_generator_replace_contents($path, $search, $replace)) {
      return FALSE;
    }
  }

  return TRUE;
}

/**
 * Replaces all occurrences of a given string in a given file.
 *
 * @param $file
 *   The file to replace all occurrences of $search in.
 * @param $search
 *   The string(s) to look for when replacing the file contents. Can be an array
 *   or a string.
 * @param $replace
 *   The string(s) to replace $search with (file contents). Can be an array or
 *   a string.
 * @param $regex
 *   (Optional) Whether $search is a regex pattern. Defaults to FALSE.
 *
 * @see root_generator_rewrite_recursive()
 * @see str_replace()
 * @see preg_replace()
 */
function root_generator_replace_contents($file, $search, $replace, $regex = FALSE) {
  // Make sure we don't choke on anything that is not a file. Also, it doesn't
  // make sense to load the file contents of an empty file.
  if (is_file($file) && filesize($file) > 0) {
    $before = file_get_contents($file);

    // Use preg_replace() if the given $search string is a regex pattern.
    if ($regex && $before != ($after = preg_replace($search, $replace, $before))) {
      file_unmanaged_save_data($after, $file, FILE_EXISTS_REPLACE);
    }
    // Otherwise simply use str_replace().
    elseif (!$regex && $before != ($after = str_replace($search, $replace, $before))) {
      file_unmanaged_save_data($after, $file, FILE_EXISTS_REPLACE);
    }
  }
}

/**
 * Recursively builds an .info file structure from an array.
 *
 * @param $array
 *   The array to build the .info file from.
 * @param $prefix
 *   (Optional) Used internally to forward the current prefix (level of nesting)
 *   for the keys.
 *
 * @return string
 *   A .info file string.
 */
function root_generator_build_info_file($array, $prefix = FALSE) {
  $info = '';

  foreach ($array as $key => $value) {
    if (is_array($value)) {
      // This is an array, let's proceed with the next level.
      $info .= root_generator_build_info_file($value, (!$prefix ? $key : "{$prefix}[{$key}]"));
    }
    else {
      // Escape all single quotes.
      $value = str_replace("'", "\'", $value);
      // Wrap the value in single quotes if it has any trailing or leading
      // whitespace.
      $value = trim($value) != $value ? "'" . $value . "'" : $value;
      // If the key is numeric remove it entirely (simple brackets are enough in
      // this case).
      $key = is_numeric($key) ? '' : $key;

      $info .= $prefix ? ("{$prefix}[" . $key .']') : $key;
      $info .= ' = ' . $value . "\n";
    }
  }

  return $info;
}

/**
 * Helper function for creating and saving a .info file.
 *
 * @param $name
 *   The name of the .info file (equivalent to the machine-readable name of the
 *   theme that the .info file belongs to).
 * @param $data
 *   The content of the .info file.
 * @param $destination
 *   (Optional) The path in which the .info file should be saved. Defaults to
 *   the path of the theme as defined by $name.
 *
 * @return bool
 *   TRUE if the .info file was successfully saved, FALSE otherwise.
 */
function root_generator_write_info_file($name, $data, $destination = NULL) {
  $destination = isset($destination) ? $destination : drupal_get_path('theme', $name);

  if (!empty($destination)) {
    $data = is_array($data) ? root_generator_build_info_file($data) : $data;
    return file_unmanaged_save_data($data, $destination . '/' . $name . '.info', FILE_EXISTS_REPLACE);
  }

  return FALSE;
}

/**
 * Creates a subtheme from any given starterkit.
 *
 * @param $subtheme
 *   An object containing all the required definitions for creating a subtheme.
 *   The available properties are:
 *
 *   - 'name': The human-readable name of the subtheme.
 *   - 'machine_name': The machine-readable name of the subtheme.
 *   - 'path': The destination that the subtheme should be placed in relative to
 *     the Drupal root.
 *   - 'starterkit': The machine-readable name of the starterkit that should be
 *     used for creating the subtheme.
 *   - 'default': (optional) Whether the subtheme should be enabled and set as
 *     the default theme after it has been created.
 *   - 'enable': (ptional) Whether the subtheme should be enabled after it has
 *     been created.
 *
 * @return bool
 *   TRUE if the subtheme could be successfully created, FALSE otherwise.
 */
function root_generator_subtheme_create($subtheme) {
  // Check whether the destination path does not exist and bail out if it does
  // so we don't delete any important data by accident.
  if (is_dir($subtheme->path)) {
    watchdog('Root Generator', t('The path @path aready exists.', array('@path' => $subtheme->path)), array(), WATCHDOG_ERROR);
    return FALSE;
  }

  // Try to prepare the destination directory for the new subtheme.
  if (!file_prepare_directory($subtheme->path, FILE_CREATE_DIRECTORY)) {
    watchdog('root', t('The Root subtheme generator was not able to create the directory @dir.', array('@dir' => $subtheme->path)), array(), WATCHDOG_ERROR);
    return FALSE;
  }

  $themes = list_themes();
  // Try to copy the starterkit to the destination path of the new subtheme.
  $source = drupal_get_path('theme', $subtheme->starterkit);
  if (!root_generator_copy_recursive($source, $subtheme->path)) {
    watchdog('root', t('The Root subtheme generator was not able to copy the starterkit @starterkit.', array('@starterkit' => $themes[$subtheme->starterkit]->info['name'])), array(), WATCHDOG_ERROR);
    return FALSE;
  }

  // Recursively rewrite the file names and contents of all the files that are
  // now in the subtheme's directory to represent the human- and
  // machine-readable names of the subtheme.
  root_generator_rewrite_recursive($subtheme->path, array($subtheme->starterkit, $themes[$subtheme->starterkit]->info['name']), array($subtheme->machine_name, $subtheme->name));

  // Parse the contents of the current .info file (provided by the starterkit).
  $subtheme->info = drupal_parse_info_file($subtheme->path . '/' . $subtheme->machine_name . '.info');

  // Unset all unneeded (and potentially hindering) properties in the .info file
  // of the subtheme.
  unset($subtheme->info['starterkit'], $subtheme->info['hidden'], $subtheme->info['locked'], $subtheme->info['project'], $subtheme->info['datestamp']);

  // Put the name and description, as well as a 'dummy' version for the new
  // subtheme in place.
  $subtheme->info['name'] = $subtheme->name;
  $subtheme->info['description'] = $subtheme->description;
  $subtheme->info['version'] = '1.x';

  // Overwrite the existing .info file with the changed values.
  root_generator_write_info_file($subtheme->machine_name, $subtheme->info, $subtheme->path);

  // Rebuild the theme caches so that we can do some final tasks.
  drupal_theme_rebuild();
  system_rebuild_theme_data();

  if (!empty($subtheme->enable) || !empty($subtheme->default)) {
    // Enable the subtheme.
    theme_enable(array($subtheme->machine_name));

    if (!empty($subtheme->default)) {
      // Make the newly created subtheme the default theme.
      variable_set('theme_default', $subtheme->machine_name);
    }
  }

  return TRUE;
}

/**
 * Retrieve an array of available sites from the sites.php.
 *
 * @return
 *   An array that contains all the available sites in a multisite environment.
 */
function root_generator_sites() {
  $sites = array();

  // Load the available sites (multisite installation) from the sites.php file
  // if it exists.
  if (file_exists(DRUPAL_ROOT . '/sites/sites.php')) {
    include DRUPAL_ROOT . '/sites/sites.php';
  }

  // The 'all' destination is always available.
  return array('all') + array_values($sites);
}

/**
 * Delete the theme settings that have been stored in the database and thereby
 * reverts them to the default settings from the .info file.
 *
 * @param $name
 *   The machine-readable name of a theme.
 */
function root_generator_revert_theme_settings($theme) {
  $themes = list_themes();

  if (isset($themes[$theme])) {
    // Delete the theme settings variable for the given theme.
    variable_del('theme_' . $theme . '_settings');

    // Rebuild the theme data for good measure.
    drupal_theme_rebuild();
    system_rebuild_theme_data();

    watchdog('root', t('The theme settings for the @theme the have been reset to their default values.', array('@theme' => $themes[$theme]->info['name'])));
    return TRUE;
  }

  return FALSE;
}

/**
 * Delete the theme settings that have been stored in the database and thereby
 * reverts them to the default settings from the .info file.
 *
 * @param $name
 *   The machine-readable name of a theme.
 */
function root_generator_export_theme_settings($theme) {
  $themes = list_themes();

  if (isset($themes[$theme])) {
    // Parse the current content of the .info file so we can append the settings
    // from the database.
    $path = drupal_get_path('theme', $theme) . '/' . $theme . '.info';
    $info = drupal_parse_info_file($path);

    // Remove the old theme settings from the .info file.
    unset($info['settings']);

    // Insert the theme settings from the database.
    if ($settings = variable_get('theme_' . $theme . '_settings')) {
      $info['settings'] = $settings;
    }

    // Write the data to the .info file of the theme.
    if (root_generator_write_info_file($theme, $info)) {
      watchdog('root', t('The theme settings for the @theme theme have been exported to the .info file of the theme.', array('@theme' => $themes[$theme]->info['name'])));
      return TRUE;
    }

    watchdog('root', t('There was an error while trying to export the theme settings for the @theme theme.', array('@theme' => $themes[$theme]->info['name'])));
  }

  return FALSE;
}

/**
 * Helper function for generating a valid machine-readable name for a theme from
 * any string.
 *
 * @param $name
 *   The string to generate the machine-readable name from.
 *
 * @return string
 *   The generated machine-readable name.
 */
function _root_generator_generate_theme_name($string) {
  // Machine-readable names have to start with a lowercase letter.
  $string = preg_replace('/^[^a-z]+/', '', strtolower($string));
  // Machine-readable may only contain alphanumeric characters and underscores.
  $string = preg_replace('/[^a-z0-9_]+/', '_', $string);
  // Trim all trailing and leading underscores.
  $string = trim($string, '_');

  return $string;
}
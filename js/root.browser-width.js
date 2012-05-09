(function ($) {

/**
 * Renders a widget for displaying the current width of the browser.
 */
Drupal.behaviors.rootBrowserWidth = {
  attach: function (context, settings) {
    $('body').once('root-browser-width', function (){
       // Add the browser width indicator.
      $width = $('<div class="root-browser-width" />').appendTo(this);

      // Bind to the window.resize event to continuously update the width.
      $(window).bind('resize.root-browser-width', function () {
        $width.text($(this).width() + 'px');
      }).trigger('resize.root-browser-width');
    });
  }
}

})(jQuery);

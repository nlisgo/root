(function ($) {

/**
 * Renders a widget for displaying the current width of the browser.
 */
Drupal.behaviors.omegaBrowserWidth = {
  attach: function (context, settings) {
    $('body').once('omega-browser-width', function (){
       // Add the browser width indicator.
      $width = $('<div class="omega-browser-width" />').appendTo(this);

      // Bind to the window.resize event to continuously update the width.
      $(window).bind('resize.omega-browser-width', function () {
        $width.text($(this).width() + 'px');
      }).trigger('resize.omega-browser-width');
    });
  }
}

})(jQuery);

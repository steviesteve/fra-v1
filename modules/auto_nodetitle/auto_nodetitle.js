// $Id: auto_nodetitle.js,v 1.1.2.1 2010/04/14 09:47:13 fago Exp $
(function ($) {

Drupal.behaviors.auto_nodetitleFieldsetSummaries = {
  attach: function (context) {
    $('fieldset#edit-auto-nodetitle', context).setSummary(function (context) {

      // Retrieve the value of the selected radio button
      var ant = $("input[@name=#edit-auto-nodetitle-ant]:checked").val();

      if (ant==0) {
        return Drupal.t('Disabled')
      }
      else if (ant==1) {
        return Drupal.t('Automatic (hide title field)')
      }
      else if (ant==2) {
        return Drupal.t('Automatic (if title empty)')
      }
    });
  }
};

})(jQuery);

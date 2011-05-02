(function ($) {
	 $(document).ready(function() {

		if($('.pagerj').length) {
		var offMax = $('.pagerj').offset().left + $('.pagerj').width();
		var offMin = $('.pagerj').offset().left;
		// put all your jQuery goodness in here.
		

		$('.views_slideshow_cycle_main').bind('slideAction',function() {
		$(".views-field-entity-id-2 a").each(function()
			{
				var hr = $(this).attr('href');
				var parent_id = $(this).parents(".views-slideshow-cycle-main-frame-row").attr('id');
				var search_str = 'views_slideshow_cycle_div_project_slider-page'
				var pager_val=parent_id.substr(search_str.length);
				var pager_num = 0;
				var selected = $('#views_slideshow_pager_field_item_project_slider-page' + pager_val);
				
			    if ($(this).is(":visible"))
			    {
				 $('.menu a[href$="'+ hr + '"]').addClass('selected');			 
					selected.fadeTo('fast', 1);	
	 			 pager_num = parseInt(pager_val.substr(pager_val.lastIndexOf('_')+1));
				if(!isNaN(pager_num) && ((selected.offset().left > offMax) || (selected.offset().left < offMin)) ) {
					$('.pagerj').jcarousel('scroll',pager_num);
				}

			    }
			    else
			    {
				$('.menu a[href$="'+ hr + '"]').removeClass('selected');
				selected.fadeTo('fast', 0.5);
					
			    }
			})
		});
		}
	});
})(jQuery);

//is(":visible") /

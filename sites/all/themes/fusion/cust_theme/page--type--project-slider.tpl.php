<!-- latest jQuery direct from google's CDN -->
		<script type="text/javascript" src="http://ajax.googleapis.com/ajax/libs/jquery/1.4/jquery.min.js"></script>
		<!-- the mousewheel plugin -->
		<script type="text/javascript" src="script/jquery.mousewheel.js"></script>
		<!-- the jScrollPane script -->

		<script type="text/javascript" src="script/jquery.jscrollpane.min.js"></script>
		<!-- scripts specific to this demo site -->
		<script type="text/javascript" src="script/demo.js"></script>
<?php print render($page['content']); ?>
<?php jcarousel_add('all_slider',array('scroll'=>1,'visible'=>1)); ?> 




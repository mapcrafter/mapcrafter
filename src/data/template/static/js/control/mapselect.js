MapSelectControl.prototype = new BaseControl("MapSelectControl");

/**
 * This control widget allows the user to select a map.
 */
function MapSelectControl() {
}

MapSelectControl.prototype.create = function(wrapper) {

	wrapper.innerHTML = '<button type="button" class="btn btn-default dropdown-toggle" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">'
	+ this.ui.getMapConfig(this.ui.getMapConfigsOrder()[0]).name
	+ ' <span class="caret"></span></button>';

	var select = document.createElement("ul");
	select.setAttribute("id", "map-select");
	select.setAttribute("class", "dropdown-menu dropdown-menu-right");

	for(var i in this.ui.getMapConfigsOrder()) {
		var type = this.ui.getMapConfigsOrder()[i];
		var option = document.createElement("li");
		var link = document.createElement("a");

		link.innerHTML = this.ui.getMapConfig(type).name;
		link.setAttribute("data-world", type);

		link.addEventListener("click", (function(ui) {
			return function(a) {
				var h = a.target.getAttribute('data-world');
				var button = document.getElementById('control-wrapper-map-select').getElementsByTagName('button')[0];
				button.innerHTML = this.ui.getMapConfig(h).name + ' <span class="caret"></span>';
				ui.setMap(h);
			}
		})(this.ui));

		option.appendChild(link);
		select.appendChild(option);
	}
	wrapper.appendChild(select);
};

MapSelectControl.prototype.getName = function() {
	return 'map-select';
};

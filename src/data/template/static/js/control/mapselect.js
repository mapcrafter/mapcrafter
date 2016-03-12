MapSelectControl.prototype = new BaseControl("MapSelectControl");

/**
 * This control widget allows the user to select a map.
 */
function MapSelectControl() {
}

MapSelectControl.prototype.create = function(wrapper) {

	wrapper.innerHTML = '<button type="button" class="btn btn-default dropdown-toggle" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">Map Select <span class="caret"></span> </button>';
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
				ui.setMap(a.target.getAttribute('data-world'));
			}
		})(this.ui));

		option.appendChild(link);
		select.appendChild(option);
	}
	wrapper.appendChild(select);
};

MapSelectControl.prototype.name = function() {
	return 'map-select';
};

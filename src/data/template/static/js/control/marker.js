MarkerControl.prototype = new BaseControl("MarkerControl");

function MarkerControl(markers) {
	this.handler = new MarkerHandler(markers);
}

MarkerControl.prototype.create = function(wrapper) {	
	var checkboxes = document.createElement("div");
	
	var groups = this.handler.getMarkerGroups();
	for (var i = 0; i < groups.length; i++) {
		var group = groups[i][0];
		var groupLabel = groups[i][1];
		
		var container = document.createElement("div");
		var checkbox = document.createElement("input");
		checkbox.setAttribute("id", "cb_group_" + group);
		checkbox.setAttribute("data-group", group);
		checkbox.setAttribute("type", "checkbox");
		checkbox.style.verticalAlign = "middle";
		checkbox.checked = true;
		
		var self = this;
		checkbox.addEventListener("change", function(checkbox) {
			var group = this.getAttribute("data-group");
			self.handler.show(group, this.checked);
		});
		
		var label = document.createElement("label");
		label.setAttribute("for", "cb_group_" + group);
		label.innerHTML = groupLabel;
		
		container.appendChild(checkbox);
		container.appendChild(label);
		checkboxes.appendChild(container);
	}

	var label = document.createElement("div");
	label.innerHTML = "Show markers:";
	wrapper.appendChild(label);
	wrapper.appendChild(checkboxes);
	
	this.ui.addHandler(this.handler);
};

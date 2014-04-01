MarkerControl.prototype = new BaseControl("MarkerControl");

function MarkerControl(markers) {
	this.handler = new MarkerHandler(markers);
}

MarkerControl.prototype.create = function(wrapper) {
	var groups = this.handler.getMarkerGroups();
	var self = this;
	
	var checkboxes = document.createElement("div");
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
		
		checkbox.addEventListener("change", function() {
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
	
	var showAll = document.createElement("a");
	showAll.setAttribute("href", "#");
	showAll.innerHTML = "Show all";
	showAll.addEventListener("click", function(event) {
		for (var i = 0; i < groups.length; i++) {
			var checkbox = document.getElementById("cb_group_" + groups[i][0]);
			checkbox.checked = true;
			self.handler.show(groups[i][0], true);
		}
		event.preventDefault()
	});
	
	var spacer = document.createElement("span");
	spacer.innerHTML = " ";
	
	var hideAll = document.createElement("a");
	hideAll.setAttribute("href", "#");
	hideAll.innerHTML = "Hide all";
	hideAll.addEventListener("click", function(event) {
		for (var i = 0; i < groups.length; i++) {
			var checkbox = document.getElementById("cb_group_" + groups[i][0]);
			checkbox.checked = false;
			self.handler.show(groups[i][0], false);
		}
		event.preventDefault()
	});
	
	var container = document.createElement("div");
	container.style.textAlign = "center";
	container.appendChild(showAll);
	container.appendChild(spacer);
	container.appendChild(hideAll);
	
	wrapper.appendChild(label);
	wrapper.appendChild(checkboxes);
	wrapper.appendChild(container);
};

MarkerControl.prototype.getHandler = function() {
	return this.handler;
};

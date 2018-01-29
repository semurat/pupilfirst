import React from "react";
import PropTypes from "prop-types";

export default class SubmitButton extends React.Component {
  constructor(props) {
    super(props);
    this.openTimelineBuilder = this.openTimelineBuilder.bind(this);
  }

  openTimelineBuilder() {
    this.props.openTimelineBuilderCB(
      this.props.target.id,
      this.props.target.timeline_event_type_id
    );
  }

  submitButtonText() {
    if (this.props.target.call_to_action) {
      return this.props.target.call_to_action;
    } else if (!this.props.target.link_to_complete) {
      return this.isPending() ? "Submit" : "Re-Submit";
    } else {
      return this.isPending() ? "Complete" : "Update";
    }
  }

  submitButtonIconClass() {
    if (this.props.target.call_to_action) {
      return "fa fa-chevron-circle-right";
    } else if (!this.props.target.link_to_complete) {
      return "fa fa-upload";
    } else {
      return "fa fa-external-link-square";
    }
  }

  isPending() {
    return this.props.target.status === "pending";
  }

  render() {
    return (
      <div className="pull-right">
        {this.props.target.link_to_complete && (
          <a
            href={this.props.target.link_to_complete}
            className="btn btn-with-icon btn-md btn-secondary text-uppercase btn-timeline-builder js-founder-dashboard__trigger-builder js-founder-dashboard__action-bar-add-event-button"
          >
            <i className={this.submitButtonIconClass()} aria-hidden="true" />
            <span>{this.submitButtonText()}</span>
          </a>
        )}
        {!this.props.target.link_to_complete && (
          <button
            onClick={this.openTimelineBuilder}
            className="btn btn-with-icon btn-md btn-secondary text-uppercase btn-timeline-builder js-founder-dashboard__trigger-builder js-founder-dashboard__action-bar-add-event-button"
          >
            <i className={this.submitButtonIconClass()} aria-hidden="true" />
            <span>{this.submitButtonText()}</span>
          </button>
        )}
      </div>
    );
  }
}

SubmitButton.propTypes = {
  target: PropTypes.object,
  openTimelineBuilderCB: PropTypes.func
};
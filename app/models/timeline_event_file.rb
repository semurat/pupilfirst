class TimelineEventFile < ActiveRecord::Base
  belongs_to :timeline_event

  mount_uploader :file, TimelineEventFileUploader

  # Used to determine whether file can be downloaded by visitor.
  def visible_to?(user)
    # Owner of event will always have visibility.
    return true if user == timeline_event.user

    # Only owner has visibility to private event (and attachments).
    return false if timeline_event.private?

    # A public file should be visible.
    return true unless private?

    # If private, check if visitor is a founder in linked startup.
    user.present? && timeline_event.startup.founders.include?(user)
  end
end

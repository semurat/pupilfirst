[%bs.raw {|require("./CoachDashboard.scss")|}];

type props = {
  coach: Coach.t,
  startups: list(Startup.t),
  timelineEvents: list(TimelineEvent.t),
  authenticityToken: string,
  emptyIconUrl: string,
};

type state = {
  selectedStartupId: option(int),
  timelineEvents: list(TimelineEvent.t),
};

type action =
  | SelectStartup(int)
  | ClearStartup
  | ReplaceTE(TimelineEvent.t);

let component = ReasonReact.reducerComponent("CoachDashboard");

let make =
    (
      ~coach,
      ~startups,
      ~timelineEvents,
      ~authenticityToken,
      ~emptyIconUrl,
      _children,
    ) => {
  ...component,
  initialState: () => {selectedStartupId: None, timelineEvents},
  reducer: (action, state) =>
    switch (action) {
    | SelectStartup(id) =>
      ReasonReact.Update({...state, selectedStartupId: Some(id)})
    | ClearStartup => ReasonReact.Update({...state, selectedStartupId: None})
    | ReplaceTE(newTE) =>
      ReasonReact.Update({
        ...state,
        timelineEvents:
          state.timelineEvents
          |> List.map(oldTE =>
               oldTE |> TimelineEvent.id == (newTE |> TimelineEvent.id) ?
                 newTE : oldTE
             ),
      })
    },
  render: ({state, send}) => {
    let selectStartupCB = id => send(SelectStartup(id));
    let clearStartupCB = () => send(ClearStartup);
    let replaceTE_CB = te => send(ReplaceTE(te));
    <div className="coach-dashboard__container container">
      <div className="row">
        <div className="col-md-4">
          {
            let pendingCount =
              state.timelineEvents
              |> TimelineEvent.verificationPending
              |> List.length;
            <SidePanel
              coach
              startups
              selectedStartupId=state.selectedStartupId
              selectStartupCB
              clearStartupCB
              pendingCount
            />;
          }
        </div>
        <div className="col-md-8">
          <TimelineEventsPanel
            timelineEvents=state.timelineEvents
            selectedStartupId=state.selectedStartupId
            replaceTE_CB
            authenticityToken
            emptyIconUrl
          />
        </div>
      </div>
    </div>;
  },
};

let decode = json =>
  Json.Decode.{
    coach: json |> field("coach", Coach.decode),
    startups: json |> field("startups", list(Startup.decode)),
    timelineEvents:
      json |> field("timelineEvents", list(TimelineEvent.decode)),
    authenticityToken: json |> field("authenticityToken", string),
    emptyIconUrl: json |> field("emptyIconUrl", string),
  };

let jsComponent =
  ReasonReact.wrapReasonForJs(
    ~component,
    jsProps => {
      let props = jsProps |> decode;
      make(
        ~coach=props.coach,
        ~startups=props.startups,
        ~timelineEvents=props.timelineEvents,
        ~authenticityToken=props.authenticityToken,
        ~emptyIconUrl=props.emptyIconUrl,
        [||],
      );
    },
  );
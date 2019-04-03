open StudentsPanel__Types;

open SchoolAdmin__Utils;

let str = ReasonReact.string;

type formVisible =
  | None
  | CreateForm
  | UpdateForm(Student.t);

type state = {
  teams: list(Team.t),
  selectedStudents: list(Student.t),
  searchString: string,
  formVisible,
  selectedLevelNumber: option(int),
  tags: list(string),
  tagsFilteredBy: list(string),
  filterVisible: bool,
};

type action =
  | UpdateTeams(list(Team.t))
  | SelectStudent(Student.t)
  | DeselectStudent(Student.t)
  | SelectAllStudents
  | DeselectAllStudents
  | UpdateSearchString(string)
  | UpdateFormVisible(formVisible)
  | UpdateSelectedLevelNumber(option(int))
  | AddNewTags(list(string))
  | AddTagFilter(string)
  | RemoveTagFilter(string)
  | ToggleFilterVisibility;

let selectedAcrossTeams = selectedStudents =>
  selectedStudents
  |> List.map(s => s |> Student.teamId)
  |> List.sort_uniq((id1, id2) => id1 - id2)
  |> List.length > 1;

let selectedPartialTeam = (selectedStudents, teams) => {
  let selectedTeam =
    teams
    |> List.find(t =>
         Team.id(t) == (selectedStudents |> List.hd |> Student.teamId)
       );
  selectedStudents
  |> List.length < (selectedTeam |> Team.students |> List.length);
};

let selectedWithinLevel = (selectedStudents, teams) => {
  let teamIds =
    selectedStudents |> List.map(student => student |> Student.teamId);
  let selectedUniqTeams =
    teams |> List.filter(team => List.mem(team |> Team.id, teamIds));
  let selectedLevelNumbers =
    selectedUniqTeams |> List.map(team => team |> Team.levelNumber);
  selectedLevelNumbers
  |> List.sort_uniq((ln1, ln2) => ln1 - ln2)
  |> List.length == 1;
};

let isGroupable = (selectedStudents, teams) =>
  selectedStudents
  |> List.length > 1
  && selectedWithinLevel(selectedStudents, teams)
  && (
    selectedAcrossTeams(selectedStudents)
    || selectedPartialTeam(selectedStudents, teams)
  );

let isMoveOutable = (selectedStudents, teams) =>
  selectedStudents
  |> List.length == 1
  && teams
  |> List.find(team =>
       team |> Team.id == (selectedStudents |> List.hd |> Student.teamId)
     )
  |> Team.students
  |> List.length > 1;

let filteredTeams = state => {
  let levelFilteredTeams =
    switch (state.selectedLevelNumber) {
    | None => state.teams
    | Some(n) =>
      state.teams |> List.filter(team => team |> Team.levelNumber == n)
    };
  let tagFilteredTeams =
    switch (state.tagsFilteredBy) {
    | [] => levelFilteredTeams
    | tags =>
      levelFilteredTeams
      |> List.filter(team =>
           tags
           |> List.for_all(tag =>
                team
                |> Team.students
                |> List.map(student => student |> Student.tags)
                |> List.flatten
                |> List.mem(tag)
              )
         )
    };
  tagFilteredTeams
  |> List.filter(team =>
       team
       |> Team.students
       |> List.map(s => s |> Student.name)
       |> List.filter(n =>
            n
            |> String.lowercase
            |> Js.String.includes(state.searchString |> String.lowercase)
          )
       |> List.length > 0
     );
};

let handleTeamUpResponse = (send, json) => {
  let teams = json |> Json.Decode.(field("teams", list(Team.decode)));
  send(UpdateTeams(teams));
  send(DeselectAllStudents);
  Notification.success("Success!", "Teams updated successfully");
};

let handleErrorCB = () => ();

let teamUp = (students, responseCB, authenticityToken) => {
  let payload = Js.Dict.empty();
  Js.Dict.set(
    payload,
    "authenticity_token",
    authenticityToken |> Js.Json.string,
  );
  Js.Dict.set(
    payload,
    "founder_ids",
    students |> List.map(s => s |> Student.id) |> Json.Encode.(list(int)),
  );
  let url = "/school/students/team_up";
  Api.create(url, payload, responseCB, handleErrorCB);
};

let component = ReasonReact.reducerComponent("SA_StudentsPanel");

let make =
    (
      ~teams,
      ~courseId,
      ~courseCoachIds,
      ~schoolCoaches,
      ~authenticityToken,
      ~levels,
      ~studentTags,
      _children,
    ) => {
  ...component,
  initialState: () => {
    teams,
    selectedStudents: [],
    searchString: "",
    formVisible: None,
    selectedLevelNumber: None,
    tagsFilteredBy: [],
    tags: studentTags,
    filterVisible: false,
  },
  reducer: (action, state) =>
    switch (action) {
    | UpdateTeams(teams) => ReasonReact.Update({...state, teams})
    | SelectStudent(student) =>
      ReasonReact.Update({
        ...state,
        selectedStudents: [student, ...state.selectedStudents],
      })
    | DeselectStudent(student) =>
      ReasonReact.Update({
        ...state,
        selectedStudents:
          state.selectedStudents
          |> List.filter(s => Student.id(s) !== Student.id(student)),
      })
    | SelectAllStudents =>
      ReasonReact.Update({
        ...state,
        selectedStudents:
          state.teams |> List.map(t => t |> Team.students) |> List.flatten,
      })
    | DeselectAllStudents =>
      ReasonReact.Update({...state, selectedStudents: []})
    | UpdateSearchString(searchString) =>
      ReasonReact.Update({...state, searchString})
    | UpdateFormVisible(formVisible) =>
      ReasonReact.Update({...state, formVisible})
    | UpdateSelectedLevelNumber(selectedLevelNumber) =>
      ReasonReact.Update({...state, selectedLevelNumber})
    | AddTagFilter(tag) =>
      ReasonReact.Update({
        ...state,
        tagsFilteredBy: [tag, ...state.tagsFilteredBy],
      })
    | RemoveTagFilter(tag) =>
      ReasonReact.Update({
        ...state,
        tagsFilteredBy: state.tagsFilteredBy |> List.filter(t => t !== tag),
      })
    | AddNewTags(tags) =>
      ReasonReact.Update({
        ...state,
        tags:
          List.append(tags, state.tags) |> List.sort_uniq(String.compare),
      })
    | ToggleFilterVisibility =>
      ReasonReact.Update({...state, filterVisible: ! state.filterVisible})
    },
  render: ({state, send}) =>
    <div className="flex flex-1 flex-col bg-grey-lightest overflow-hidden">
      {
        let closeFormCB = () => send(UpdateFormVisible(None));
        let submitFormCB = (teams, tags) => {
          send(UpdateTeams(teams));
          send(AddNewTags(tags));
          send(UpdateFormVisible(None));
        };
        switch (state.formVisible) {
        | None => ReasonReact.null
        | CreateForm =>
          <SA_StudentsPanel_CreateForm
            courseId
            closeFormCB
            submitFormCB
            studentTags=state.tags
            authenticityToken
          />
        | UpdateForm(student) =>
          let teamCoachIds =
            state.teams
            |> List.find(team => Team.id(team) == Student.teamId(student))
            |> Team.coachIds;
          <SA_StudentsPanel_UpdateForm
            student
            studentTags=state.tags
            teamCoachIds
            courseCoachIds
            schoolCoaches
            closeFormCB
            submitFormCB
            authenticityToken
          />;
        };
      }
      <div
        className="border-b px-6 py-2 bg-white items-center justify-between z-20">
        <div className="inline-block relative w-64">
          <select
            onChange=(
              event => {
                let level_number = ReactEvent.Form.target(event)##value;
                send(
                  UpdateSelectedLevelNumber(
                    level_number == "all" ?
                      None : Some(level_number |> int_of_string),
                  ),
                );
              }
            )
            value=(
              switch (state.selectedLevelNumber) {
              | None => "all"
              | Some(n) => n |> string_of_int
              }
            )
            className="block appearance-none w-full bg-white border border-grey-light hover:border-grey px-4 py-2 pr-8 rounded leading-tight leading-normal focus:outline-none">
            <option value="all"> ("All levels" |> str) </option>
            (
              levels
              |> List.map(level =>
                   <option
                     key=(level |> Level.number |> string_of_int)
                     value=(level |> Level.number |> string_of_int)>
                     (
                       "Level "
                       ++ (level |> Level.number |> string_of_int)
                       ++ ": "
                       ++ (level |> Level.name)
                       |> str
                     )
                   </option>
                 )
              |> Array.of_list
              |> ReasonReact.array
            )
          </select>
          <div
            className="pointer-events-none absolute pin-y pin-r flex items-center px-2 text-grey-darker">
            <Icon kind=Icon.Down size="3" />
          </div>
        </div>
      </div>
      <div className="overflow-y-scroll">
        <div className="px-3">
          <div
            className="max-w-lg h-16 bg-white mx-auto relative rounded border-b p-4 mt-3 w-full flex items-center justify-between">
            <div className="flex">
              <label className="flex items-center leading-tight mr-4 my-auto">
                <input
                  className="leading-tight"
                  type_="checkbox"
                  htmlFor="selected-students"
                  checked=(state.selectedStudents |> List.length > 0)
                  onChange=(
                    state.selectedStudents |> List.length > 0 ?
                      _e => send(DeselectAllStudents) :
                      (_e => send(SelectAllStudents))
                  )
                />
                <span
                  id="selected-students"
                  className="ml-2 text-sm text-grey-dark">
                  {
                    let selectedCount = state.selectedStudents |> List.length;
                    let studentCount =
                      filteredTeams(state)
                      |> List.map(team => team |> Team.students)
                      |> List.flatten
                      |> List.length;
                    selectedCount > 0 ?
                      (selectedCount |> string_of_int) ++ " selected" |> str :
                      (studentCount |> string_of_int) ++ " students" |> str;
                  }
                </span>
              </label>
            </div>
            <div className="flex">
              (
                false ?
                  <button
                    className="bg-grey-lighter hover:bg-grey-light hover:text-grey-darker focus:outline-none text-grey-dark text-sm font-semibold py-2 px-4 rounded inline-flex items-center mx-2">
                    ("Add tags" |> str)
                  </button> :
                  ReasonReact.null
              )
              (
                isGroupable(state.selectedStudents, state.teams) ?
                  <button
                    onClick=(
                      _e =>
                        teamUp(
                          state.selectedStudents,
                          handleTeamUpResponse(send),
                          authenticityToken,
                        )
                    )
                    className="bg-transparent hover:bg-purple-dark focus:outline-none text-purple-dark text-sm font-semibold hover:text-white py-2 px-4 border border-puple hover:border-transparent rounded">
                    ("Group as Team" |> str)
                  </button> :
                  ReasonReact.null
              )
              (
                isMoveOutable(state.selectedStudents, state.teams) ?
                  <button
                    onClick=(
                      _e =>
                        teamUp(
                          state.selectedStudents,
                          handleTeamUpResponse(send),
                          authenticityToken,
                        )
                    )
                    className="bg-transparent hover:bg-purple-dark focus:outline-none text-purple-dark text-sm font-semibold hover:text-white py-2 px-4 border border-puple hover:border-transparent rounded">
                    ("Move out from Team" |> str)
                  </button> :
                  ReasonReact.null
              )
              (
                state.selectedStudents |> List.length > 0 ?
                  ReasonReact.null :
                  <button
                    onClick=(_e => send(UpdateFormVisible(CreateForm)))
                    className="hover:bg-purple-dark text-purple-dark font-semibold hover:text-white focus:outline-none border border-dashed border-blue hover:border-transparent flex items-center px-2 py-1 rounded-lg cursor-pointer">
                    <i className="material-icons mr-2">
                      ("add_circle_outline" |> str)
                    </i>
                    <h5 className="font-semibold ml-2">
                      ("Add New Students" |> str)
                    </h5>
                  </button>
              )
            </div>
          </div>
          <div
            className="max-w-lg bg-white mx-auto relative rounded rounded-b-none border-b px-4 py-3 mt-3 w-full">
            <div className="flex items-center justify-between">
              <input
                type_="search"
                className="bg-white border rounded-lg block w-64 text-sm appearance-none leading-normal mr-2 px-3 py-2"
                placeholder="Search by student name..."
                value=state.searchString
                onChange=(
                  event =>
                    send(
                      UpdateSearchString(
                        ReactEvent.Form.target(event)##value,
                      ),
                    )
                )
              />
              <div
                onClick=(_e => send(ToggleFilterVisibility))
                className="flex text-indigo items-center">
                <p className="text-sm font-semibold mr-1">
                  (
                    (state.filterVisible ? "Hide" : "Show") ++ " Filters" |> str
                  )
                </p>
                <i className="material-icons md-48">
                  (
                    (state.filterVisible ? "expand_less" : "expand_more") |> str
                  )
                </i>
              </div>
            </div>
          </div>
        </div>
        <div className="flex bg-grey-lightest pb-6">
          <div className="flex flex-col max-w-lg mx-auto w-full">
            (
              state.filterVisible && state.tags |> List.length > 0 ?
                <div className="px-4 py-3 border-b bg-grey-lighter shadow">
                  <div className="flex flex-col pt-2">
                    <div className="mb-1 text-sm"> ("Filters:" |> str) </div>
                    <SA_StudentsPanel_SearchableTagList
                      unselectedTags=(
                        state.tags
                        |> List.filter(tag =>
                             ! (state.tagsFilteredBy |> List.mem(tag))
                           )
                      )
                      selectedTags=state.tagsFilteredBy
                      addTagCB=(tag => send(AddTagFilter(tag)))
                      removeTagCB=(tag => send(RemoveTagFilter(tag)))
                      allowNewTags=false
                    />
                  </div>
                </div> :
                ReasonReact.null
            )
            <div className="w-full py-3 rounded-b-lg">
              (
                filteredTeams(state) |> List.length > 0 ?
                  filteredTeams(state)
                  |> List.sort((team1, team2) =>
                       Team.id(team2) - Team.id(team1)
                     )
                  |> List.map(team => {
                       let isSingleFounder =
                         team |> Team.students |> List.length == 1;
                       <div
                         key=(team |> Team.id |> string_of_int)
                         id=(team |> Team.name)
                         className=(
                           "student-team-container flex items-center shadow bg-white rounded-lg mb-4 overflow-hidden"
                           ++ (
                             isSingleFounder ? " hover:bg-grey-lightest" : ""
                           )
                         )>
                         <div className="flex-1 w-3/5">
                           (
                             team
                             |> Team.students
                             |> List.map(student => {
                                  let isChecked =
                                    state.selectedStudents
                                    |> List.mem(student);
                                  <div
                                    key=(
                                      student |> Student.id |> string_of_int
                                    )
                                    id=(student |> Student.name)
                                    className="student-team__card cursor-pointer flex items-center bg-white hover:bg-grey-lightest">
                                    <div className="flex-1 w-3/5">
                                      <div className="flex items-center">
                                        <label
                                          className="block text-grey leading-tight font-bold px-4 py-5"
                                          htmlFor=(
                                            (student |> Student.name)
                                            ++ "_checkbox"
                                          )>
                                          <input
                                            className="leading-tight"
                                            type_="checkbox"
                                            id=(
                                              (student |> Student.name)
                                              ++ "_checkbox"
                                            )
                                            checked=isChecked
                                            onChange=(
                                              isChecked ?
                                                _e =>
                                                  send(
                                                    DeselectStudent(student),
                                                  ) :
                                                (
                                                  _e =>
                                                    send(
                                                      SelectStudent(student),
                                                    )
                                                )
                                            )
                                          />
                                        </label>
                                        <a
                                          className="flex flex-1 items-center py-4 pr-4"
                                          id=(
                                            (student |> Student.name)
                                            ++ "_edit"
                                          )
                                          onClick=(
                                            _e =>
                                              send(
                                                UpdateFormVisible(
                                                  UpdateForm(student),
                                                ),
                                              )
                                          )>
                                          <img
                                            className="w-10 h-10 rounded-full mr-4"
                                            src=(student |> Student.avatarUrl)
                                          />
                                          <div
                                            className="text-sm flex flex-col">
                                            <p
                                              className=(
                                                "text-black font-semibold inline-block "
                                                ++ (
                                                  state.searchString
                                                  |> String.length > 0
                                                  && student
                                                  |> Student.name
                                                  |> String.lowercase
                                                  |> Js.String.includes(
                                                       state.searchString
                                                       |> String.lowercase,
                                                     ) ?
                                                    "bg-yellow-light" : ""
                                                )
                                              )>
                                              (student |> Student.name |> str)
                                            </p>
                                            <div className="flex">
                                              (
                                                student
                                                |> Student.tags
                                                |> List.map(tag =>
                                                     <div
                                                       key=tag
                                                       className="border border-indigo rounded mt-2 mr-1 py-1 px-2 text-xs text-indigo">
                                                       (tag |> str)
                                                     </div>
                                                   )
                                                |> Array.of_list
                                                |> ReasonReact.array
                                              )
                                            </div>
                                          </div>
                                        </a>
                                      </div>
                                    </div>
                                  </div>;
                                })
                             |> Array.of_list
                             |> ReasonReact.array
                           )
                         </div>
                         <div className="flex w-2/5 items-center">
                           <div className="w-3/5 py-4 px-3">
                             (
                               isSingleFounder ?
                                 ReasonReact.null :
                                 <div className="students-team--name mb-5">
                                   <p className="mb-1 text-xs">
                                     ("Team" |> str)
                                   </p>
                                   <h4> (team |> Team.name |> str) </h4>
                                 </div>
                             )
                             <div className="coaches-avatar-group">
                               <p className="mb-2 text-xs">
                                 ("Coaches" |> str)
                               </p>
                               <div className="flex items-center">
                                 {
                                   let teamCoachIds =
                                     List.append(
                                       courseCoachIds,
                                       team |> Team.coachIds,
                                     );
                                   let teamCoaches =
                                     schoolCoaches
                                     |> List.filter(coach =>
                                          teamCoachIds
                                          |> List.exists(teamCoachId =>
                                               teamCoachId == Coach.id(coach)
                                             )
                                        );
                                   teamCoaches
                                   |> List.map(coach =>
                                        <img
                                          key=(coach |> Coach.avatarUrl)
                                          className="w-6 h-6 rounded-full mr-2"
                                          src=(coach |> Coach.avatarUrl)
                                          alt=(
                                            "Avatar of "
                                            ++ (coach |> Coach.name)
                                          )
                                        />
                                      )
                                   |> Array.of_list
                                   |> ReasonReact.array;
                                 }
                               </div>
                             </div>
                           </div>
                           <div className="w-2/5 text-center">
                             <span
                               className="inline-flex flex-col rounded bg-indigo-lightest px-2 py-1">
                               <div className="text-xs">
                                 ("Level" |> str)
                               </div>
                               <div className="text-xl font-semibold">
                                 (
                                   team
                                   |> Team.levelNumber
                                   |> string_of_int
                                   |> str
                                 )
                               </div>
                             </span>
                           </div>
                         </div>
                       </div>;
                     })
                  |> Array.of_list
                  |> ReasonReact.array :
                  <div className="shadow bg-white rounded-lg mb-4 p-4">
                    ("No student matches your search/filter criteria." |> str)
                  </div>
              )
            </div>
          </div>
        </div>
      </div>
    </div>,
};

type props = {
  teams: list(Team.t),
  courseId: int,
  courseCoachIds: list(int),
  schoolCoaches: list(Coach.t),
  levels: list(Level.t),
  studentTags: list(string),
  authenticityToken: string,
};

let decode = json =>
  Json.Decode.{
    teams: json |> field("teams", list(Team.decode)),
    courseId: json |> field("courseId", int),
    courseCoachIds: json |> field("courseCoachIds", list(int)),
    levels: json |> field("levels", list(Level.decode)),
    studentTags: json |> field("studentTags", list(string)),
    schoolCoaches: json |> field("schoolCoaches", list(Coach.decode)),
    authenticityToken: json |> field("authenticityToken", string),
  };

let jsComponent =
  ReasonReact.wrapReasonForJs(
    ~component,
    jsProps => {
      let props = jsProps |> decode;
      make(
        ~teams=props.teams,
        ~courseId=props.courseId,
        ~courseCoachIds=props.courseCoachIds,
        ~schoolCoaches=props.schoolCoaches,
        ~levels=props.levels,
        ~studentTags=props.studentTags,
        ~authenticityToken=props.authenticityToken,
        [||],
      );
    },
  );
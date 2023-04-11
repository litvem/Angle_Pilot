# DIT639 Cyber Physical Systems and Systems of Systems
## Group 13

[![pipeline status](https://git.chalmers.se/courses/dit638/students/2023-group-13/badges/main/pipeline.svg)](https://git.chalmers.se/courses/dit638/students/2023-group-13/-/commits/main)
[![coverage report](https://git.chalmers.se/courses/dit638/students/2023-group-13/badges/main/coverage.svg)](https://git.chalmers.se/courses/dit638/students/2023-group-13/-/commits/main)
[![Latest Release](https://git.chalmers.se/courses/dit638/students/2023-group-13/-/badges/release.svg)](https://git.chalmers.se/courses/dit638/students/2023-group-13/-/releases)

[Team Code of Conduct](./code-of-conduct.md)

## Local repository setup
### <ins>Prerequisites</ins>
- Install **Git** on your computer (follow [instructions](https://www.atlassian.com/git/tutorials/install-git#linux))
- Install **Git Bash** (<ins>for Windows users only</ins>) (follow [instructions](https://www.atlassian.com/git/tutorials/git-bash#:~:text=How%20to%20install%20Git%20Bash,open%20to%20execute%20Git%20Bash))
- Setup **SSH key** (follow [instructions](https://blog.robertelder.org/what-is-ssh/))

### <ins>Setup steps</ins>
1. Open terminal (MacOS) or Git Bash (on Windows) 
2. Navigate to desired folder to store project files ([introductions on how to navigate between directories](https://ubuntu.com/tutorials/command-line-for-beginners#1-overview))
3. Type in ``git clone git@git.chalmers.se:courses/dit638/students/2023-group-13.git`` and hit ENTER.

## Procedure for adding new feature
1. Selected internal Product Owner will create a set of requirements / user stories, each with a set of acceptance criteria
2. During group meetings the team will discuss and agree on the feasibility of the proposed feature and assign tasks to group members
    - If the feature is deemed features is deemed feasible, proceed to step 3
   - Else, rescope the requirement / user story
3. Assign tasks to group members all together during one meeting
4. The selected internal Product Owner will create a Trello card and GitLab issue following the setup template
5. One of the assigned team members will create a new feature branch in the remote repository
6. The assigned team members will start feature development
7. Once all acceptance criteria for the requirement / user story have been met, create a merge request using an appropriate merge request template and request a review from a team member
8. The reviewee performs a code review and tests the implementation on their own machine in a docker container
   - If the implementation works and the code meets the criteria, the reviewee approves the merge request and deletes the source branch
   - Else, depending on the severity of the fault, the team will discuss possible solutions and decide on one. Reassign another team member if necessary

## Procedure for fixing unexpected behavior in existing features
1. The feature developer locates the bug 
2. The feature developer creates an issue on GitLab with the ‘bug’ label, explaining the problem and how to recreate it
3. The feature developer attempt to fix the cause of the fault performed by the developer of related feature
   - If the developer is not able to fix the bug, ask for other team members assistance
   - If the team is not able to fix the bug, ask for TAs assistance
4. If the fault could be resolved, create a merge request and request code review. If it can’t be resolved, roll back to the previous version that didn’t have the bug
5. Discuss what to do next with the rest of the team

## Rules for commits
1. If applicable, start commit message subject with a reference to related issue (e.g., #3 Fix a bug…)
2. Separate the commit subject fro body with a blank line
3. Limit the commit subject line to 50 characters
4. The commit subject should be imperative and start with a verb (e.g. Fix a bug…)
5. Capitalize the commit subject line
6. Do not end the commit subject line with a period (i.e., dot)
7. Limit commit message body to 80 characters
8. Use the commit message body to explain WHAT and WHY not HOW

## Contributors
- Robert Einer					
- Emma Litvin					
- Ossian Ålund			
- Bao Quan Lindgren						
- Khaled Adel Saleh Mohammed Al-Baadani

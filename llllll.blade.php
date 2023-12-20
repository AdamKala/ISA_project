@if ($editingId == $item['id'] || $editingId == 0)
                                                <x-button
                                                    wire:click.prevent="toggleEdit({{ $item['id'] }})"
                                                    class="mr-3 bg-red-600">Zrušit
                                                </x-button>
                                                <x-button
                                                    wire:click.prevent="saveEdit()"
                                                    class="bg-[#32cd32]">
                                                    Uložit
                                                </x-button>
                                            @else
                                                @if(Auth::id() == $item['created_by'] || Auth::id() == 0)
                                                    <button
                                                        wire:click.prevent="toggleEdit({{ $item['id'] }})"
                                                        class="fa-solid fa-pen fa-2xl ml-3"
                                                        style="color: #228B22;"/>
                                                    <button
                                                        wire:click.prevent="deletComment({{ $item['id'] }})"
                                                        class="fa-solid fa-2xl fa-trash ml-3"
                                                        style="color: #ff0000;"/>
                                                @endif
                                            @endif
{{--                                    @if(Auth::id() != null)--}}
{{--                                        <div class="flex justify-center">--}}
{{--                                            <x-input wire:model.debounce.500ms="subcommentAdd" id="subcommentAdd"--}}
{{--                                                     type="text" class="w-full"/>--}}
{{--                                            <x-button--}}
{{--                                                wire:click.prevent="createSubcomment({{ $threadId }}, {{$item['id']}})"--}}
{{--                                                class="mr-3 bg-blue-600">Okomentovat--}}
{{--                                            </x-button>--}}
{{--                                        </div>--}}
{{--                                    @endif--}}

                                            {{--                                        <div--}}
                                            {{--                                            class="grid grid-cols-3 grid-flow-col ml-6 mr-6 p-6 shadow-xl bg-white">--}}
                                            {{--                                            <h2 class="mt-2 text-l font-semibold text-gray-900">--}}
                                            {{--                                                Autor {{ $subcom['author'] }}</h2>--}}
                                            {{--                                            <span style="white-space: initial"--}}
                                            {{--                                                  class="mt-2 text-l text-gray-900">{{ $subcom['text'] }}</span>--}}

                                            {{--                                            <div class="flex my-auto justify-end">--}}
                                            {{--                                                @php--}}
                                            {{--                                                    $userInGroup = \App\Models\group_user::where('user_id', Auth::id())->where('group_id', $item->id)->exists();--}}
                                            {{--                                                @endphp--}}

                                            {{--                                                @if ($editingSubId == $subcom['id'] || $editingSubId == 0)--}}
                                            {{--                                                    <x-button--}}
                                            {{--                                                        wire:click.prevent="toggleSubEdit({{ $subcom['id'] }})"--}}
                                            {{--                                                        class="mr-3 bg-red-600">Zrušit--}}
                                            {{--                                                    </x-button>--}}
                                            {{--                                                    <x-button wire:click.prevent="saveSubEdit()"--}}
                                            {{--                                                              class="bg-[#32cd32]">Uložit--}}
                                            {{--                                                    </x-button>--}}
                                            {{--                                                @else--}}
                                            {{--                                                    @if(Auth::id() == $subcom['created_by'] || Auth::id() == 0)--}}
                                            {{--                                                        <button--}}
                                            {{--                                                            wire:click.prevent="toggleSubEdit({{ $subcom['id'] }})"--}}
                                            {{--                                                            class="fa-solid fa-pen fa-2xl ml-3"--}}
                                            {{--                                                            style="color: #228B22;"/>--}}
                                            {{--                                                        <button--}}
                                            {{--                                                            wire:click.prevent="deleteSubcomment({{ $subcom['id'] }})"--}}
                                            {{--                                                            class="fa-solid fa-2xl fa-trash ml-3"--}}
                                            {{--                                                            style="color: #ff0000;"/>--}}
                                            {{--                                                    @endif--}}
                                            {{--                                                @endif--}}
                                            {{--                                            </div>--}}
                                            {{--                                        </div> --}}
